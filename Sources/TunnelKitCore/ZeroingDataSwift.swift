import Foundation
import CoreFoundation

/// Pure-Swift re-implementation of ZeroingData, registered with the ObjC runtime
/// as "ZeroingData" so CTunnelKitOpenVPNProtocol C code can call it without a
/// separate ObjC implementation file.
@objc(ZeroingData)
@objcMembers
public final class ZeroingData: NSObject {
    private var _bytes: UnsafeMutablePointer<UInt8>
    private var _count: Int

    public var bytes: UnsafePointer<UInt8> { UnsafePointer(_bytes) }
    public var mutableBytes: UnsafeMutablePointer<UInt8> { _bytes }
    public var count: Int { _count }

    private static func alloc(_ n: Int) -> UnsafeMutablePointer<UInt8> {
        let cap = max(n, 1)
        let ptr = UnsafeMutablePointer<UInt8>.allocate(capacity: cap)
        ptr.initialize(repeating: 0, count: cap)
        return ptr
    }

    public override init() {
        _count = 0
        _bytes = ZeroingData.alloc(0)
        super.init()
    }

    @objc(initWithCount:)
    public init(count: Int) {
        _count = count
        _bytes = ZeroingData.alloc(count)
        super.init()
    }

    @objc(initWithBytes:count:)
    public init(bytes: UnsafePointer<UInt8>?, count: Int) {
        _count = count
        _bytes = ZeroingData.alloc(count)
        if let b = bytes, count > 0 { _bytes.initialize(from: b, count: count) }
        super.init()
    }

    @objc(initWithUInt8:)
    public init(uInt8: UInt8) {
        _count = 1
        _bytes = ZeroingData.alloc(1)
        _bytes[0] = uInt8
        super.init()
    }

    @objc(initWithUInt16:)
    public init(uInt16: UInt16) {
        _count = 2
        _bytes = ZeroingData.alloc(2)
        _bytes[0] = UInt8(uInt16 & 0xff)
        _bytes[1] = UInt8(uInt16 >> 8)
        super.init()
    }

    @objc(initWithData:)
    public init(data: NSData) {
        let n = data.length
        _count = n
        _bytes = ZeroingData.alloc(n)
        if n > 0 { _bytes.initialize(from: data.bytes.assumingMemoryBound(to: UInt8.self), count: n) }
        super.init()
    }

    @objc(initWithData:offset:count:)
    public init(data: NSData, offset: Int, count: Int) {
        _count = count
        _bytes = ZeroingData.alloc(count)
        if count > 0 { _bytes.initialize(from: data.bytes.assumingMemoryBound(to: UInt8.self).advanced(by: offset), count: count) }
        super.init()
    }

    @objc(initWithString:nullTerminated:)
    public init(string: String, nullTerminated: Bool) {
        let utf8Bytes = Array(string.utf8)
        let len = utf8Bytes.count
        _count = len + (nullTerminated ? 1 : 0)
        _bytes = ZeroingData.alloc(_count)
        super.init()
        // Phase 2: safe to use self after super.init()
        for i in 0..<len { _bytes[i] = utf8Bytes[i] }
        if nullTerminated { _bytes[len] = 0 }
    }

    fileprivate init(nocopy ptr: UnsafeMutablePointer<UInt8>, count: Int) {
        _bytes = ptr; _count = count
        super.init()
    }

    deinit {
        _bytes.initialize(repeating: 0, count: max(_count, 1))
        _bytes.deallocate()
    }

    public func appendData(_ other: ZeroingData) {
        let n = _count + other._count
        let nb = ZeroingData.alloc(n)
        if _count > 0 { nb.initialize(from: _bytes, count: _count) }
        if other._count > 0 { nb.advanced(by: _count).initialize(from: other._bytes, count: other._count) }
        _bytes.initialize(repeating: 0, count: max(_count, 1)); _bytes.deallocate()
        _bytes = nb; _count = n
    }

    public func removeUntilOffset(_ until: Int) {
        let n = _count - until
        let nb = ZeroingData.alloc(n)
        if n > 0 { nb.initialize(from: _bytes.advanced(by: until), count: n) }
        _bytes.initialize(repeating: 0, count: max(_count, 1)); _bytes.deallocate()
        _bytes = nb; _count = n
    }

    public func zero() { _bytes.initialize(repeating: 0, count: max(_count, 1)) }

    public func appendingData(_ other: ZeroingData) -> ZeroingData {
        let n = _count + other._count
        let nb = ZeroingData.alloc(n)
        if _count > 0 { nb.initialize(from: _bytes, count: _count) }
        if other._count > 0 { nb.advanced(by: _count).initialize(from: other._bytes, count: other._count) }
        return ZeroingData(nocopy: nb, count: n)
    }

    @objc(withOffset:count:)
    public func withBytesOffset(_ offset: Int, count: Int) -> ZeroingData {
        let nb = ZeroingData.alloc(count)
        if count > 0 { nb.initialize(from: _bytes.advanced(by: offset), count: count) }
        return ZeroingData(nocopy: nb, count: count)
    }


    // MARK: - Swift-name aliases (call sites use these names)

    /// Alias for appendData — matches Swift naming used in EncryptionBridge etc.
    public func append(_ other: ZeroingData) { appendData(other) }

    /// Alias for appendingData — matches Swift naming used in EncryptionBridge etc.
    public func appending(_ other: ZeroingData) -> ZeroingData { appendingData(other) }

    /// Alias for withBytesOffset — matches Swift naming used in EncryptionBridge etc.
    /// (The @objc selector is already "withOffset:count:" for ObjC callers.)
    public func withOffset(_ offset: Int, count: Int) -> ZeroingData {
        withBytesOffset(offset, count: count)
    }

    @objc(UInt16ValueFromOffset:)
    public func uint16Value(fromOffset from: Int) -> UInt16 {
        return UInt16(_bytes[from]) | (UInt16(_bytes[from + 1]) << 8)
    }

    @objc(networkUInt16ValueFromOffset:)
    public func networkUInt16Value(fromOffset from: Int) -> UInt16 {
        return CFSwapInt16BigToHost(UInt16(_bytes[from]) | (UInt16(_bytes[from + 1]) << 8))
    }

    @objc(nullTerminatedStringFromOffset:)
    public func nullTerminatedString(fromOffset from: Int) -> String? {
        for i in from..<_count where _bytes[i] == 0 {
            return String(bytes: UnsafeBufferPointer(start: _bytes.advanced(by: from), count: i - from), encoding: .ascii)
        }
        return nil
    }

    @objc(isEqualToData:)
    public func isEqual(toData data: NSData) -> Bool {
        guard data.length == _count else { return false }
        return _count == 0 || memcmp(_bytes, data.bytes, _count) == 0
    }

    public func toData() -> NSData { NSData(bytes: _bytes, length: _count) }

    public func toHex() -> String {
        (0..<_count).map { String(format: "%02x", _bytes[$0]) }.joined()
    }
}


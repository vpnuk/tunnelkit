// TunnelKitBridge.h — fully self-contained ObjC bridging header for TunnelKit.
// All ObjC type declarations needed by TunnelKitOpenVPNCore and
// TunnelKitOpenVPNProtocol inlined here. Zero deps beyond Foundation.
//
// Location: ~/Desktop/tvos-source/TunnelKitBridge.h
// Xcode runs the Swift compiler with CWD = VPNClient.xcodeproj/
// so the Package.swift flag ../TunnelKitBridge.h resolves here stably,
// regardless of DerivedData UUID or cache resets.

#import <Foundation/Foundation.h>
#import <stdint.h>

NS_ASSUME_NONNULL_BEGIN

// ── Enums ─────────────────────────────────────────────────────────────────────

#ifndef CompressionAlgorithmNative_defined
#define CompressionAlgorithmNative_defined
typedef NS_ENUM(NSInteger, CompressionAlgorithmNative) {
    CompressionAlgorithmNativeDisabled,
    CompressionAlgorithmNativeLZO,
    CompressionAlgorithmNativeOther
};
#endif

#ifndef CompressionFramingNative_defined
#define CompressionFramingNative_defined
typedef NS_ENUM(NSInteger, CompressionFramingNative) {
    CompressionFramingNativeDisabled,
    CompressionFramingNativeCompLZO,
    CompressionFramingNativeCompress,
    CompressionFramingNativeCompressV2
};
#endif

#ifndef XORMethodNative_defined
#define XORMethodNative_defined
typedef NS_ENUM(NSInteger, XORMethodNative) {
    XORMethodNativeNone,
    XORMethodNativeMask,
    XORMethodNativePtrPos,
    XORMethodNativeReverse,
    XORMethodNativeObfuscate
};
#endif

typedef NS_ENUM(NSInteger, OpenVPNErrorCode) {
    OpenVPNErrorCodeCryptoRandomGenerator       = 101,
    OpenVPNErrorCodeCryptoHMAC                  = 102,
    OpenVPNErrorCodeCryptoEncryption            = 103,
    OpenVPNErrorCodeCryptoAlgorithm             = 104,
    OpenVPNErrorCodeTLSCARead                   = 201,
    OpenVPNErrorCodeTLSCAUse                    = 202,
    OpenVPNErrorCodeTLSCAPeerVerification       = 203,
    OpenVPNErrorCodeTLSClientCertificateRead    = 204,
    OpenVPNErrorCodeTLSClientCertificateUse     = 205,
    OpenVPNErrorCodeTLSClientKeyRead            = 206,
    OpenVPNErrorCodeTLSClientKeyUse             = 207,
    OpenVPNErrorCodeTLSHandshake                = 210,
    OpenVPNErrorCodeTLSServerCertificate        = 211,
    OpenVPNErrorCodeTLSServerEKU                = 212,
    OpenVPNErrorCodeTLSServerHost               = 213,
    OpenVPNErrorCodeDataPathOverflow            = 301,
    OpenVPNErrorCodeDataPathPeerIdMismatch      = 302,
    OpenVPNErrorCodeDataPathCompression         = 303
};

extern NSString *const OpenVPNErrorDomain;
extern NSString *const OpenVPNErrorKey;

static inline NSError *OpenVPNErrorWithCode(OpenVPNErrorCode code) {
    return [NSError errorWithDomain:OpenVPNErrorDomain code:code userInfo:nil];
}

typedef NS_ENUM(uint8_t, PacketCode) {
    PacketCodeSoftResetV1           = 0x03,
    PacketCodeControlV1             = 0x04,
    PacketCodeAckV1                 = 0x05,
    PacketCodeDataV1                = 0x06,
    PacketCodeHardResetClientV2     = 0x07,
    PacketCodeHardResetServerV2     = 0x08,
    PacketCodeDataV2                = 0x09,
    PacketCodeUnknown               = 0xff
};

// ── Packet constants & macros ─────────────────────────────────────────────────

#define PacketOpcodeLength          ((NSInteger)1)
#define PacketIdLength              ((NSInteger)4)
#define PacketSessionIdLength       ((NSInteger)8)
#define PacketAckLengthLength       ((NSInteger)1)
#define PacketPeerIdLength          ((NSInteger)3)
#define PacketPeerIdDisabled        ((uint32_t)0xffffffu)
#define PacketReplayIdLength        ((NSInteger)4)
#define PacketReplayTimestampLength ((NSInteger)4)

#define DataPacketNoCompress        0xfa
#define DataPacketNoCompressSwap    0xfb
#define DataPacketLZOCompress       0x66
#define DataPacketV2Indicator       0x50
#define DataPacketV2Uncompressed    0x00

extern const uint8_t DataPacketPingData[16];

static inline void PacketOpcodeGet(const uint8_t *from, PacketCode *_Nullable code, uint8_t *_Nullable key)
{
    if (code) { *code = (PacketCode)(*from >> 3); }
    if (key)  { *key  = *from & 0b111; }
}

static inline int PacketHeaderSet(uint8_t *to, PacketCode code, uint8_t key, const uint8_t *_Nullable sessionId)
{
    *(uint8_t *)to = (code << 3) | (key & 0b111);
    int offset = PacketOpcodeLength;
    if (sessionId) {
        memcpy(to + offset, sessionId, PacketSessionIdLength);
        offset += PacketSessionIdLength;
    }
    return offset;
}

static inline int PacketHeaderSetDataV2(uint8_t *to, uint8_t key, uint32_t peerId)
{
    *(uint32_t *)to = ((PacketCodeDataV2 << 3) | (key & 0b111)) | htonl(peerId & 0xffffff);
    return PacketOpcodeLength + PacketPeerIdLength;
}

static inline int PacketHeaderGetDataV2PeerId(const uint8_t *from)
{
    return ntohl(*(const uint32_t *)from & 0xffffff00);
}

static inline void PacketSwap(uint8_t *ptr, NSInteger len1, NSInteger len2)
{
    uint8_t buf1[len1]; uint8_t buf2[len2];
    memcpy(buf1, ptr, len1); memcpy(buf2, ptr + len1, len2);
    memcpy(ptr, buf2, len2); memcpy(ptr + len2, buf1, len1);
}

static inline void PacketSwapCopy(uint8_t *dst, NSData *src, NSInteger len1, NSInteger len2)
{
    NSCAssert(src.length >= len1 + len2, @"src is smaller than expected");
    memcpy(dst, src.bytes + len1, len2);
    memcpy(dst + len2, src.bytes, len1);
    const NSInteger preambleLength = len1 + len2;
    memcpy(dst + preambleLength, src.bytes + preambleLength, src.length - preambleLength);
}

// ── CryptoMacros ──────────────────────────────────────────────────────────────

#define TUNNEL_CRYPTO_SUCCESS(ret) (ret > 0)
#define TUNNEL_CRYPTO_TRACK_STATUS(ret) if (ret > 0) ret =
#define TUNNEL_CRYPTO_RETURN_STATUS(ret)\
if (ret <= 0) {\
    if (error) { *error = OpenVPNErrorWithCode(OpenVPNErrorCodeCryptoEncryption); }\
    return NO;\
}\
return YES;

// ── DataPathCrypto typedefs & macros ──────────────────────────────────────────

#define DATA_PATH_ENCRYPT_INIT(peerId) \
    const BOOL hasPeerId = (peerId != PacketPeerIdDisabled); \
    int headerLength = PacketOpcodeLength; \
    if (hasPeerId) { headerLength += PacketPeerIdLength; }

#define DATA_PATH_DECRYPT_INIT(packet) \
    const uint8_t *ptr = packet.bytes; \
    PacketCode code; \
    PacketOpcodeGet(ptr, &code, NULL); \
    uint32_t peerId = PacketPeerIdDisabled; \
    const BOOL hasPeerId = (code == PacketCodeDataV2); \
    int headerLength = PacketOpcodeLength; \
    if (hasPeerId) { \
        headerLength += PacketPeerIdLength; \
        if (packet.length < headerLength) { return NO; } \
        peerId = PacketHeaderGetDataV2PeerId(ptr); \
    }

typedef void (^DataPathAssembleBlock)(uint8_t *packetDest, NSInteger *packetLengthOffset, NSData *payload);
typedef BOOL (^DataPathParseBlock)(uint8_t *payload,
                                   NSInteger *payloadOffset,
                                   uint8_t *header,
                                   NSInteger *headerLength,
                                   const uint8_t *packet,
                                   NSInteger packetLength,
                                   NSError **error);

// ── Forward declarations ──────────────────────────────────────────────────────

// ── CTunnelKitCore types ─────────────────────────────────────────────────────

void *allocate_safely(size_t size);
size_t safe_crypto_capacity(size_t size, size_t overhead);

@protocol CompressionProvider
- (nullable NSData *)compressedDataWithData:(NSData *)data error:(NSError **)error;
- (nullable NSData *)decompressedDataWithData:(NSData *)data error:(NSError **)error;
- (nullable NSData *)decompressedDataWithBytes:(const uint8_t *)bytes length:(NSInteger)length error:(NSError **)error;
@end

@interface ZeroingData : NSObject
@property (nonatomic, readonly) const uint8_t *bytes;
@property (nonatomic, readonly) uint8_t *mutableBytes;
@property (nonatomic, readonly) NSInteger count;
- (instancetype)initWithCount:(NSInteger)count;
- (instancetype)initWithBytes:(nullable const uint8_t *)bytes count:(NSInteger)count;
- (instancetype)initWithUInt8:(uint8_t)uint8;
- (instancetype)initWithUInt16:(uint16_t)uint16;
- (instancetype)initWithData:(NSData *)data;
- (instancetype)initWithData:(NSData *)data offset:(NSInteger)offset count:(NSInteger)count;
- (instancetype)initWithString:(NSString *)string nullTerminated:(BOOL)nullTerminated;
- (void)appendData:(ZeroingData *)other;
- (void)removeUntilOffset:(NSInteger)until;
- (void)zero;
- (ZeroingData *)appendingData:(ZeroingData *)other;
- (ZeroingData *)withOffset:(NSInteger)offset count:(NSInteger)count;
- (uint16_t)UInt16ValueFromOffset:(NSInteger)from;
- (uint16_t)networkUInt16ValueFromOffset:(NSInteger)from;
- (nullable NSString *)nullTerminatedStringFromOffset:(NSInteger)from;
- (BOOL)isEqualToData:(NSData *)data;
- (NSData *)toData;
- (NSString *)toHex;
@end

@interface LZOFactory : NSObject
+ (BOOL)isSupported;
+ (id<CompressionProvider>)create;
@end

// ── DataPath protocols ────────────────────────────────────────────────────────

@protocol DataPathChannel
- (uint32_t)peerId;
- (void)setPeerId:(uint32_t)peerId;
- (NSInteger)encryptionCapacityWithLength:(NSInteger)length;
@end

@protocol DataPathEncrypter <DataPathChannel>
- (void)assembleDataPacketWithBlock:(nullable DataPathAssembleBlock)block
                           packetId:(uint32_t)packetId
                            payload:(NSData *)payload
                               into:(uint8_t *)packetBytes
                             length:(NSInteger *)packetLength;
- (nullable NSData *)encryptedDataPacketWithKey:(uint8_t)key
                                       packetId:(uint32_t)packetId
                                    packetBytes:(const uint8_t *)packetBytes
                                   packetLength:(NSInteger)packetLength
                                          error:(NSError **)error;
@end

@protocol DataPathDecrypter <DataPathChannel>
- (BOOL)decryptDataPacket:(NSData *)packet
                     into:(uint8_t *)packetBytes
                   length:(NSInteger *)packetLength
                 packetId:(uint32_t *)packetId
                    error:(NSError **)error;
- (nullable NSData *)parsePayloadWithBlock:(nullable DataPathParseBlock)block
                         compressionHeader:(uint8_t *)compressionHeader
                               packetBytes:(uint8_t *)packetBytes
                              packetLength:(NSInteger)packetLength
                                     error:(NSError **)error;
@end

// ── Crypto protocols ──────────────────────────────────────────────────────────

typedef struct {
    const uint8_t *_Nullable iv;
    NSInteger ivLength;
    const uint8_t *_Nullable ad;
    NSInteger adLength;
    BOOL forTesting;
} CryptoFlags;

@protocol Encrypter
- (void)configureEncryptionWithCipherKey:(nullable ZeroingData *)cipherKey
                                 hmacKey:(nullable ZeroingData *)hmacKey;
- (int)digestLength;
- (int)tagLength;
- (NSInteger)encryptionCapacityWithLength:(NSInteger)length;
- (BOOL)encryptBytes:(const uint8_t *)bytes
              length:(NSInteger)length
                dest:(uint8_t *)dest
          destLength:(NSInteger *)destLength
               flags:(const CryptoFlags *_Nullable)flags
               error:(NSError **)error;
- (id<DataPathEncrypter>)dataPathEncrypter;
@end

@protocol Decrypter
- (void)configureDecryptionWithCipherKey:(nullable ZeroingData *)cipherKey
                                 hmacKey:(nullable ZeroingData *)hmacKey;
- (int)digestLength;
- (int)tagLength;
- (NSInteger)encryptionCapacityWithLength:(NSInteger)length;
- (BOOL)decryptBytes:(const uint8_t *)bytes
              length:(NSInteger)length
                dest:(uint8_t *)dest
          destLength:(NSInteger *)destLength
               flags:(const CryptoFlags *_Nullable)flags
               error:(NSError **)error;
- (BOOL)verifyBytes:(const uint8_t *)bytes
             length:(NSInteger)length
              flags:(const CryptoFlags *_Nullable)flags
              error:(NSError **)error;
- (id<DataPathDecrypter>)dataPathDecrypter;
@end

// ── Classes ───────────────────────────────────────────────────────────────────

@interface ReplayProtector : NSObject
- (BOOL)isReplayedPacketId:(uint32_t)packetId;
@end

@interface TLSBox : NSObject
@property (nonatomic, assign) NSInteger securityLevel;
extern const NSInteger TLSBoxMaxBufferLength;
extern NSString *const TLSBoxPeerVerificationErrorNotification;
extern const NSInteger TLSBoxDefaultSecurityLevel;
+ (nullable NSString *)md5ForCertificatePath:(NSString *)path error:(NSError **)error;
+ (nullable NSString *)decryptedPrivateKeyFromPath:(NSString *)path passphrase:(NSString *)passphrase error:(NSError **)error;
+ (nullable NSString *)decryptedPrivateKeyFromPEM:(NSString *)pem passphrase:(NSString *)passphrase error:(NSError **)error;
- (instancetype)initWithCAPath:(nonnull NSString *)caPath
             clientCertificate:(nullable NSString *)clientCertificatePEM
                     clientKey:(nullable NSString *)clientKeyPEM
                     checksEKU:(BOOL)checksEKU
                 checksSANHost:(BOOL)checksSANHost
                      hostname:(nullable NSString *)hostname;
- (BOOL)startWithError:(NSError **)error;
- (nullable NSData *)pullCipherTextWithError:(NSError **)error;
- (BOOL)pullRawPlainText:(uint8_t *)text length:(NSInteger *)length error:(NSError **)error;
- (BOOL)putCipherText:(NSData *)text error:(NSError **)error;
- (BOOL)putRawCipherText:(const uint8_t *)text length:(NSInteger)length error:(NSError **)error;
- (BOOL)putPlainText:(NSString *)text error:(NSError **)error;
- (BOOL)putRawPlainText:(const uint8_t *)text length:(NSInteger)length error:(NSError **)error;
- (BOOL)isConnected;
@end

@interface CryptoBox : NSObject
+ (NSString *)version;
+ (BOOL)preparePRNGWithSeed:(const uint8_t *)seed length:(NSInteger)length;
- (instancetype)initWithCipherAlgorithm:(nullable NSString *)cipherAlgorithm
                        digestAlgorithm:(nullable NSString *)digestAlgorithm;
- (BOOL)configureWithCipherEncKey:(nullable ZeroingData *)cipherEncKey
                     cipherDecKey:(nullable ZeroingData *)cipherDecKey
                       hmacEncKey:(nullable ZeroingData *)hmacEncKey
                       hmacDecKey:(nullable ZeroingData *)hmacDecKey
                            error:(NSError **)error;
+ (BOOL)hmacWithDigestName:(NSString *)digestName
                    secret:(const uint8_t *)secret
              secretLength:(NSInteger)secretLength
                      data:(const uint8_t *)data
                dataLength:(NSInteger)dataLength
                      hmac:(uint8_t *)hmac
                hmacLength:(NSInteger *)hmacLength
                     error:(NSError **)error;
- (id<Encrypter>)encrypter;
- (id<Decrypter>)decrypter;
- (NSInteger)digestLength;
- (NSInteger)tagLength;
@end

@interface CryptoAEAD : NSObject <Encrypter, Decrypter>
- (instancetype)initWithCipherName:(NSString *)cipherName;
@end

@interface DataPathCryptoAEAD : NSObject <DataPathEncrypter, DataPathDecrypter>
@property (nonatomic, assign) uint32_t peerId;
- (instancetype)initWithCrypto:(CryptoAEAD *)crypto;
@end

@interface CryptoCBC : NSObject <Encrypter, Decrypter>
- (instancetype)initWithCipherName:(nullable NSString *)cipherName digestName:(NSString *)digestName;
@end

@interface DataPathCryptoCBC : NSObject <DataPathEncrypter, DataPathDecrypter>
@property (nonatomic, assign) uint32_t peerId;
- (instancetype)initWithCrypto:(CryptoCBC *)crypto;
@end

@interface CryptoCTR : NSObject <Encrypter, Decrypter>
- (instancetype)initWithCipherName:(nullable NSString *)cipherName digestName:(NSString *)digestName;
@end

@interface DataPath : NSObject
@property (nonatomic, assign) uint32_t maxPacketId;
- (instancetype)initWithEncrypter:(id<DataPathEncrypter>)encrypter
                        decrypter:(id<DataPathDecrypter>)decrypter
                           peerId:(uint32_t)peerId
               compressionFraming:(CompressionFramingNative)compressionFraming
             compressionAlgorithm:(CompressionAlgorithmNative)compressionAlgorithm
                       maxPackets:(NSInteger)maxPackets
             usesReplayProtection:(BOOL)usesReplayProtection;
- (nullable NSArray<NSData *> *)encryptPackets:(NSArray<NSData *> *)packets key:(uint8_t)key error:(NSError **)error;
- (nullable NSArray<NSData *> *)decryptPackets:(NSArray<NSData *> *)packets keepAlive:(nullable bool *)keepAlive error:(NSError **)error;
@end

// ── MSS ───────────────────────────────────────────────────────────────────────

void MSSFix(uint8_t *data, NSInteger data_len);

// ── XOR inline functions ──────────────────────────────────────────────────────

static inline void xor_mask(uint8_t *dst, const uint8_t *src, NSData *xorMask, size_t length)
{
    if (xorMask.length > 0) {
        for (size_t i = 0; i < length; ++i) {
            dst[i] = src[i] ^ ((uint8_t *)(xorMask.bytes))[i % xorMask.length];
        }
        return;
    }
    memcpy(dst, src, length);
}

static inline void xor_ptrpos(uint8_t *dst, const uint8_t *src, size_t length)
{
    for (size_t i = 0; i < length; ++i) { dst[i] = src[i] ^ (i + 1); }
}

static inline void xor_reverse(uint8_t *dst, const uint8_t *src, size_t length)
{
    size_t start = 1, end = length - 1; uint8_t temp = 0;
    dst[0] = src[0];
    while (start < end) {
        temp = src[start]; dst[start] = src[end]; dst[end] = temp;
        start++; end--;
    }
    if (start == end) { dst[start] = src[start]; }
}

static inline void xor_memcpy(uint8_t *dst, NSData *src, XORMethodNative method, NSData *mask, BOOL outbound)
{
    const uint8_t *source = (uint8_t *)src.bytes;
    switch (method) {
        case XORMethodNativeNone:
            memcpy(dst, source, src.length); break;
        case XORMethodNativeMask:
            xor_mask(dst, source, mask, src.length); break;
        case XORMethodNativePtrPos:
            xor_ptrpos(dst, source, src.length); break;
        case XORMethodNativeReverse:
            xor_reverse(dst, source, src.length); break;
        case XORMethodNativeObfuscate:
            if (outbound) {
                xor_ptrpos(dst, source, src.length);
                xor_reverse(dst, dst, src.length);
                xor_ptrpos(dst, dst, src.length);
                xor_mask(dst, dst, mask, src.length);
            } else {
                xor_mask(dst, source, mask, src.length);
                xor_ptrpos(dst, dst, src.length);
                xor_reverse(dst, dst, src.length);
                xor_ptrpos(dst, dst, src.length);
            }
            break;
    }
}

// ── PacketStream ──────────────────────────────────────────────────────────────

@interface PacketStream : NSObject
+ (NSArray<NSData *> *)packetsFromInboundStream:(NSData *)stream
                                          until:(NSInteger *)until
                                      xorMethod:(XORMethodNative)xorMethod
                                        xorMask:(nullable NSData *)xorMask;
+ (NSData *)outboundStreamFromPacket:(NSData *)packet
                           xorMethod:(XORMethodNative)xorMethod
                             xorMask:(nullable NSData *)xorMask;
+ (NSData *)outboundStreamFromPackets:(NSArray<NSData *> *)packets
                            xorMethod:(XORMethodNative)xorMethod
                              xorMask:(nullable NSData *)xorMask;
@end

// ── ControlPacket ─────────────────────────────────────────────────────────────

@interface ControlPacket : NSObject
- (instancetype)initWithCode:(PacketCode)code
                         key:(uint8_t)key
                   sessionId:(NSData *)sessionId
                    packetId:(uint32_t)packetId
                     payload:(nullable NSData *)payload;
- (instancetype)initWithKey:(uint8_t)key
                  sessionId:(NSData *)sessionId
                     ackIds:(NSArray<NSNumber *> *)ackIds
         ackRemoteSessionId:(NSData *)ackRemoteSessionId;
@property (nonatomic, assign, readonly) PacketCode code;
@property (nonatomic, readonly) BOOL isAck;
@property (nonatomic, assign, readonly) uint8_t key;
@property (nonatomic, strong, readonly) NSData *sessionId;
@property (nonatomic, strong) NSArray<NSNumber *> *_Nullable ackIds;
@property (nonatomic, strong) NSData *_Nullable ackRemoteSessionId;
@property (nonatomic, assign, readonly) uint32_t packetId;
@property (nonatomic, strong, readonly) NSData *_Nullable payload;
@property (nonatomic, strong) NSDate *_Nullable sentDate;
- (NSData *)serialized;
@end

@interface ControlPacket (Authentication)
- (nullable NSData *)serializedWithAuthenticator:(id<Encrypter>)auth
                                        replayId:(uint32_t)replayId
                                       timestamp:(uint32_t)timestamp
                                           error:(NSError * _Nullable __autoreleasing *)error;
@end

@interface ControlPacket (Encryption)
- (nullable NSData *)serializedWithEncrypter:(id<Encrypter>)encrypter
                                    replayId:(uint32_t)replayId
                                   timestamp:(uint32_t)timestamp
                                    adLength:(NSInteger)adLength
                                       error:(NSError * _Nullable __autoreleasing *)error;
@end

NS_ASSUME_NONNULL_END

// swift-tools-version: 5.9
import PackageDescription

// NOTE: TunnelKitBridge.h must exist at ~/Desktop/tvos-source/TunnelKitBridge.h
// Content: #import "_OVPNBridge.h"
// Xcode runs the Swift compiler with CWD = VPNClient.xcodeproj/, so ../TunnelKitBridge.h
// resolves to the project root — a stable path that does not depend on DerivedData UUID.

let package = Package(
    name: "TunnelKit",
    platforms: [
        .iOS(.v15),
        .macOS(.v12),
        .tvOS(.v17)
    ],
    products: [
        .library(name: "TunnelKit", targets: ["TunnelKit"]),
        .library(name: "TunnelKitOpenVPN", targets: ["TunnelKitOpenVPN"]),
        .library(name: "TunnelKitOpenVPNAppExtension", targets: ["TunnelKitOpenVPNAppExtension"]),
        .library(name: "TunnelKitLZO", targets: ["TunnelKitLZO"])
    ],
    dependencies: [
        .package(url: "https://github.com/SwiftyBeaver/SwiftyBeaver", from: "1.9.0"),
        .package(url: "https://github.com/passepartoutvpn/openssl-apple", from: "3.2.105")
    ],
    targets: [
        .target(
            name: "TunnelKit",
            dependencies: ["TunnelKitCore", "TunnelKitManager"]
        ),
        .target(
            name: "TunnelKitCore",
            dependencies: ["__TunnelKitUtils", "SwiftyBeaver"]
        ),
        .target(
            name: "TunnelKitManager",
            dependencies: ["SwiftyBeaver"]
        ),
        .target(
            name: "TunnelKitAppExtension",
            dependencies: ["TunnelKitCore"]
        ),
        .target(
            name: "TunnelKitOpenVPN",
            dependencies: ["TunnelKitOpenVPNCore", "TunnelKitOpenVPNManager"]
        ),
        // _OVPNBridge: C target whose include/ directory gets added to the Swift compiler
        // header search paths when listed as a dependency. Swift code does NOT import
        // this as a module. Instead -import-objc-header reads TunnelKitBridge.h directly
        // (bypasses module PCM compilation — workaround for Xcode 26.5 tvOS PCM failure).
        .target(
            name: "_OVPNBridge",
            dependencies: [],
            publicHeadersPath: "include"
        ),
        .target(
            name: "TunnelKitOpenVPNCore",
            dependencies: [
                "TunnelKitCore",
                "_OVPNBridge"
            ],
            swiftSettings: [
                // ../TunnelKitBridge.h resolves relative to CWD=VPNClient.xcodeproj/
                // => ~/Desktop/tvos-source/TunnelKitBridge.h (stable, no DerivedData UUID needed)
                .unsafeFlags(["-import-objc-header", "../TunnelKitBridge.h"])
            ]
        ),
        .target(
            name: "TunnelKitOpenVPNManager",
            dependencies: ["TunnelKitManager", "TunnelKitOpenVPNCore"]
        ),
        .target(
            name: "TunnelKitOpenVPNProtocol",
            dependencies: [
                "TunnelKitOpenVPNCore",
                "CTunnelKitOpenVPNProtocol",
                "_OVPNBridge"
            ],
            swiftSettings: [
                .unsafeFlags(["-import-objc-header", "../TunnelKitBridge.h"])
            ]
        ),
        .target(
            name: "TunnelKitOpenVPNAppExtension",
            dependencies: [
                "TunnelKitAppExtension",
                "TunnelKitOpenVPNCore",
                "TunnelKitOpenVPNManager",
                "TunnelKitOpenVPNProtocol"
            ],
            swiftSettings: [
                .unsafeFlags(["-import-objc-header", "../TunnelKitBridge.h"])
            ]
        ),
        .target(
            name: "TunnelKitLZO",
            dependencies: [],
            exclude: [
                "lib/COPYING",
                "lib/Makefile",
                "lib/README.LZO",
                "lib/testmini.c"
            ]
        ),
        // C targets — compiled and linked; NOT imported as Swift modules.
        .target(
            name: "CTunnelKitCore",
            dependencies: [],
            exclude: ["ZeroingData.m"]
        ),
        .target(
            name: "CTunnelKitOpenVPNCore",
            dependencies: []
        ),
        .target(
            name: "CTunnelKitOpenVPNProtocol",
            dependencies: [
                "CTunnelKitCore",
                "CTunnelKitOpenVPNCore",
                "openssl-apple"
            ],
            cSettings: [
                .headerSearchPath("../CTunnelKitCore/include"),
                .headerSearchPath("../CTunnelKitOpenVPNCore/include")
            ]
        ),
        .target(
            name: "__TunnelKitUtils",
            dependencies: []
        ),
        .testTarget(
            name: "TunnelKitCoreTests",
            dependencies: ["TunnelKitCore"],
            exclude: [
                "RandomTests.swift",
                "RawPerformanceTests.swift",
                "RoutingTests.swift"
            ]
        ),
        .testTarget(
            name: "TunnelKitOpenVPNTests",
            dependencies: [
                "TunnelKitOpenVPNCore",
                "TunnelKitOpenVPNAppExtension",
                "TunnelKitLZO"
            ],
            exclude: [
                "DataPathPerformanceTests.swift",
                "EncryptionPerformanceTests.swift"
            ],
            resources: [.process("Resources")]
        ),
        .testTarget(
            name: "TunnelKitLZOTests",
            dependencies: ["TunnelKitCore", "TunnelKitLZO"]
        )
    ]
)


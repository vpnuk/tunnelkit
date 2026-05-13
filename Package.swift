// swift-tools-version: 5.9
import PackageDescription

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
            // CTunnelKitCore removed: ZeroingData is now a pure-Swift @objc class
            // in ZeroingDataSwift.swift, eliminating the C module import issue.
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
        .target(
            name: "TunnelKitOpenVPNCore",
            dependencies: [
                "TunnelKitCore",
                "CTunnelKitOpenVPNCore",
                "CTunnelKitOpenVPNProtocol"
            ]
        ),
        .target(
            name: "TunnelKitOpenVPNManager",
            dependencies: ["TunnelKitManager", "TunnelKitOpenVPNCore"]
        ),
        .target(
            name: "TunnelKitOpenVPNProtocol",
            dependencies: ["TunnelKitOpenVPNCore", "CTunnelKitOpenVPNProtocol"]
        ),
        .target(
            name: "TunnelKitOpenVPNAppExtension",
            dependencies: [
                "TunnelKitAppExtension",
                "TunnelKitOpenVPNCore",
                "TunnelKitOpenVPNManager",
                "TunnelKitOpenVPNProtocol"
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
        // C targets
        .target(
            name: "CTunnelKitCore",
            dependencies: [],
            // ZeroingData.m excluded: implementation is now the Swift @objc class in TunnelKitCore.
            // LZOFactory.m and Allocation.m remain for CTunnelKitOpenVPNProtocol C code.
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
                // Allocation.h, ZeroingData.h, LZOFactory.h, CompressionProvider.h
                .headerSearchPath("../CTunnelKitCore/include"),
                // Errors.h, XORMethodNative.h, CompressionAlgorithmNative.h, CompressionFramingNative.h
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


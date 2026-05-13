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
        // _OVPNBridge: single self-contained C module replacing CTunnelKitOpenVPNCore +
        // CTunnelKitOpenVPNProtocol for Swift imports. Has no deps beyond Foundation so
        // the module PCM compiles trivially. ObjC implementations are still in
        // CTunnelKitCore/CTunnelKitOpenVPNProtocol and linked at the app/extension level.
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
            ]
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
        // C targets — compiled and linked but no longer imported as Swift modules.
        // Swift code uses _OVPNBridge for type info instead.
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


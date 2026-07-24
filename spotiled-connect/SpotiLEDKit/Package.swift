// swift-tools-version: 5.10
import PackageDescription

let package = Package(
    name: "SpotiLEDKit",
    platforms: [.iOS(.v17), .macOS(.v14)],
    products: [
        .library(name: "SpotiLEDKit", targets: ["SpotiLEDKit"])
    ],
    dependencies: [
        .package(url: "https://github.com/Kolos65/Mockable", from: "0.6.4")
    ],
    targets: [
        .target(
            name: "SpotiLEDKit",
            dependencies: [
                .product(name: "Mockable", package: "Mockable")
            ],
            swiftSettings: [.define("MOCKING", .when(configuration: .debug))]
        ),
        .testTarget(
            name: "SpotiLEDKitTests",
            dependencies: [
                "SpotiLEDKit",
                .product(name: "Mockable", package: "Mockable")
            ],
            swiftSettings: [.define("MOCKING", .when(configuration: .debug))]
        )
    ]
)

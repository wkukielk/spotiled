// swift-tools-version: 5.10
import PackageDescription

let package = Package(
    name: "SpotiLEDProvisioning",
    platforms: [.iOS(.v17)],
    products: [
        .library(name: "SpotiLEDProvisioning", targets: ["SpotiLEDProvisioning"])
    ],
    dependencies: [
        .package(url: "https://github.com/espressif/esp-idf-provisioning-ios.git", from: "3.1.0"),
        .package(url: "https://github.com/Kolos65/Mockable", from: "0.6.4")
    ],
    targets: [
        .target(
            name: "SpotiLEDProvisioning",
            dependencies: [
                .product(name: "ESPProvision", package: "esp-idf-provisioning-ios"),
                .product(name: "Mockable", package: "Mockable")
            ],
            swiftSettings: [.define("MOCKING", .when(configuration: .debug))]
        ),
        .testTarget(
            name: "SpotiLEDProvisioningTests",
            dependencies: [
                "SpotiLEDProvisioning",
                .product(name: "Mockable", package: "Mockable")
            ],
            swiftSettings: [.define("MOCKING", .when(configuration: .debug))]
        )
    ]
)

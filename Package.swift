// swift-tools-version:5.3
// The swift-tools-version declares the minimum version of Swift required to build this package.
import PackageDescription

let package = Package(
    name: "Swift-TouCAN",
    products: [
        // Products define the executables and libraries a package produces, and make them visible to other packages.
        .library(
            name: "Swift-TouCAN",
            targets: [
                "CTouCAN",
                "TouCAN",
            ]
        )
    ],
    dependencies: [
        // Dependencies declare other packages that this package depends on.
        // .package(url: /* package url */, from: "1.0.0"),
    ],
    targets: [
        // Targets are the basic building blocks of a package. A target can define a module or a test suite.
        // Targets can depend on other targets in this package, and on products in packages this package depends on.
        .target(
            name: "CTouCAN",
            dependencies: [],
            cSettings: [
                CSetting.headerSearchPath("."),
                CSetting.headerSearchPath("privateInclude"),

                CSetting.define("OPTION_CAN_2_0_ONLY=0"),
                CSetting.define("OPTION_CANAPI_TOUCAN_DYLIB=1"),
                CSetting.define("OPTION_CANAPI_DRIVER=1"),
                CSetting.define("OPTION_CANAPI_RETVALS=1"),
                CSetting.define("OPTION_CANAPI_COMPANIONS=1"),
                CSetting.define("OPTION_MACCAN_LOGGER=1"),
                CSetting.define("OPTION_MACCAN_MULTICHANNEL=0"),
                CSetting.define("OPTION_MACCAN_PIPE_TIMEOUT=0"),
                CSetting.define("OPTION_MACCAN_DEBUG_LEVEL=5"),
                CSetting.define("OPTION_MACCAN_INSTRUMENTATION=0"),
                CSetting.define("OPTION_CANAPI_DEBUG_LEVEL=5"),
                CSetting.define("OPTION_CANAPI_INSTRUMENTATION=0"),
            ]
        ),
        .target(
            name: "TouCAN",
            dependencies: ["CTouCAN"],
            path: "Sources/SwiftTouCAN"
        ),
        .testTarget(
            name: "TouCAN-Tests",
            dependencies: ["TouCAN"]
        )
    ],
    cxxLanguageStandard: .cxx14
)

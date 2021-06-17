// swift-tools-version:5.3
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "MacCAN-TouCAN",
    platforms: [.macOS(.v11)],
    products: [
        // Products define the executables and libraries a package produces, and make them visible to other packages.
        .library(
            name: "MacCAN-TouCAN",
            targets: [
                "TouCAN",
                "CTouCAN"]
        ),
    ],
    dependencies: [
        // Dependencies declare other packages that this package depends on.
        // .package(url: /* package url */, from: "1.0.0"),
    ],
    targets: [
        // Targets are the basic building blocks of a package. A target can define a module or a test suite.
        // Targets can depend on other targets in this package, and on products in packages this package depends on.
        .target(
            name: "TouCAN",
            dependencies: ["CTouCAN"],
            path: "Sources/TouCAN_Swift"
        ),
        .target(
            name: "CTouCAN",
            dependencies: [],
            path: "Sources",
            exclude: [
                "TouCAN_Swift/CANAPI.swift",
                "TouCAN_Swift/TouCAN.swift",
                "MacCAN_Core/README.md",
                "MacCAN_Core/LICENSE.BSD-2-Clause",
                "MacCAN_Core/LICENSE.GPL-3.0-or-later",
                "CANAPI/README.md",
                "CANAPI/LICENSE.BSD-2-Clause",
                "CANAPI/LICENSE.GPL-3.0-or-later",
                "include/README.md"
            ],
            sources: [
                "TouCAN.cpp",
                "TouCAN_Driver/TouCAN_USB.c",
                "TouCAN_Wrapper/can_api.cpp",
                "MacCAN_Core/MacCAN.cpp",
                "MacCAN_Core/MacCAN_MsgQueue.c",
                "MacCAN_Core/MacCAN_IOUsbKit.c",
                "MacCAN_Core/MacCAN_Devices.c",
                "MacCAN_Core/MacCAN_Debug.c",
                "CANAPI/can_btr.c",
                "CANAPI/can_msg.c"
            ],
            cSettings: [
                CSetting.headerSearchPath("."),
                CSetting.headerSearchPath("./CANAPI"),
                CSetting.headerSearchPath("./MacCAN_Core"),
                CSetting.headerSearchPath("./TouCAN_Driver"),
                CSetting.headerSearchPath("./TouCAN_Wrapper"),
                CSetting.define("OPTION_CAN_2_0_ONLY=0"),
                CSetting.define("OPTION_TOUCAN_DYLIB=1"),
                CSetting.define("OPTION_CANAPI_TOUCAN_DYLIB=0"),
                CSetting.define("OPTION_CANAPI_DRIVER=1"),
                CSetting.define("OPTION_CANAPI_RETVALS=1"),
                CSetting.define("OPTION_CANAPI_COMPANIONS=1"),
                CSetting.define("OPTION_MACCAN_LOGGER=0"),
                CSetting.define("OPTION_MACCAN_MULTICHANNEL=0"),
                CSetting.define("OPTION_MACCAN_PIPE_TIMEOUT=1"),
                CSetting.define("OPTION_MACCAN_DEBUG_LEVEL=0"),
                CSetting.define("OPTION_MACCAN_INSTRUMENTATION=0"),
                CSetting.define("OPTION_CANAPI_DEBUG_LEVEL=0"),
                CSetting.define("OPTION_CANAPI_INSTRUMENTATION=0"),
                CSetting.headerSearchPath("./include"),  // dummy build no. for SPM
            ]
        ),
        .testTarget(
            name: "SwiftTests",
            dependencies: ["TouCAN"]
        ),
    ],
    cxxLanguageStandard: .cxx14
)

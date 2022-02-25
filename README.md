### macOS&reg; User-Space Driver for TouCAN USB Interfaces from Rusoku

_Copyright &copy; 2020-2022  Uwe Vogt, UV Software, Berlin (info@mac-can.com)_

# Running CAN on Mac&reg;

_Running CAN on Mac_ is the mission of the MacCAN project.
The macOS driver for TouCAN USB interfaces from [Rusoku](https://www.rusoku.com) is based on _MacCAN-Core_ which is an abstraction (or rather a wrapper) of Apple´s IOUsbKit to create USB user-space drivers for CAN interfaces from various vendors under macOS.

## MacCAN-TouCAN

This repo contains the source code for the _MacCAN-TouCAN_ driver and several alternatives to build dynamic libraries for macOS,
either as a C++ class library ([_libTouCAN_](#libTouCAN)),
or as a _CAN API V3_ driver library ([_libUVCANTOU_](#libUVCANTOU)),
or as a _VSCP CANAL_ compatible library ([_libCANAL_](#libCANAL)),
as well as my beloved CAN utilities [`can_moni`](#can_moni) and [`can_test`](#can_test),
and some C/C++, Swift, and Python example programs.


### TouCAN API

```C++
/// \name   TouCAN API
/// \brief  MacCAN driver for Rusoku TouCAN USB interfaces
/// \note   See CCanApi for a description of the overridden methods
/// \{
class CTouCAN : public CCanApi {
public:
    // constructor / destructor
    CTouCAN();
    ~CTouCAN();

    // CCanApi overrides
    static bool GetFirstChannel(SChannelInfo &info, void *param = NULL);
    static bool GetNextChannel(SChannelInfo &info, void *param = NULL);

    static CANAPI_Return_t ProbeChannel(int32_t channel, const CANAPI_OpMode_t &opMode, const void *param, EChannelState &state);
    static CANAPI_Return_t ProbeChannel(int32_t channel, const CANAPI_OpMode_t &opMode, EChannelState &state);

    CANAPI_Return_t InitializeChannel(int32_t channel, const CANAPI_OpMode_t &opMode, const void *param = NULL);
    CANAPI_Return_t TeardownChannel();
    CANAPI_Return_t SignalChannel();

    CANAPI_Return_t StartController(CANAPI_Bitrate_t bitrate);
    CANAPI_Return_t ResetController();

    CANAPI_Return_t WriteMessage(CANAPI_Message_t message, uint16_t timeout = 0U);
    CANAPI_Return_t ReadMessage(CANAPI_Message_t &message, uint16_t timeout = CANREAD_INFINITE);

    CANAPI_Return_t GetStatus(CANAPI_Status_t &status);
    CANAPI_Return_t GetBusLoad(uint8_t &load);

    CANAPI_Return_t GetBitrate(CANAPI_Bitrate_t &bitrate);
    CANAPI_Return_t GetBusSpeed(CANAPI_BusSpeed_t &speed);

    CANAPI_Return_t GetProperty(uint16_t param, void *value, uint32_t nbyte);
    CANAPI_Return_t SetProperty(uint16_t param, const void *value, uint32_t nbyte);

    char *GetHardwareVersion();  // (for compatibility reasons)
    char *GetFirmwareVersion();  // (for compatibility reasons)
    static char *GetVersion();  // (for compatibility reasons)

    static CANAPI_Return_t MapIndex2Bitrate(int32_t index, CANAPI_Bitrate_t &bitrate);
    static CANAPI_Return_t MapString2Bitrate(const char *string, CANAPI_Bitrate_t &bitrate);
    static CANAPI_Return_t MapBitrate2String(CANAPI_Bitrate_t bitrate, char *string, size_t length);
    static CANAPI_Return_t MapBitrate2Speed(CANAPI_Bitrate_t bitrate, CANAPI_BusSpeed_t &speed);
};
/// \}
```

### Build Targets

_Important note_: To build any of the following build targets run the `build_no.sh` script to generate a pseudo build number.
```
uv-pc013mac:~ eris$ cd ~/Projects/MacCAN/TouCAN
uv-pc013mac:TouCAN eris$ ./build_no.sh
```
Repeat this step after each `git commit`, `git pull`, `git clone`, etc.

Then you can build all targets by typing the usual commands:
```
uv-pc013mac:~ eris$ cd ~/Projects/MacCAN/TouCAN
uv-pc013mac:TouCAN eris$ make clean
uv-pc013mac:TouCAN eris$ make all
uv-pc013mac:TouCAN eris$ sudo make install
uv-pc013mac:TouCAN eris$
```
_(The version number of the library can be adapted by editing the appropriated `Makefile` and changing the variable `VERSION` accordingly. Don´t forget to set the version number also in the source files.)_

#### libTouCAN

___libTouCAN___ is a dynamic library with a CAN API V3 compatible application programming interface for use in __C++__ applications.
See header file `TouCAN.h` for a description of all class members.

#### libUVCANTOU

___libUVCANTOU___ is a dynamic library with a CAN API V3 compatible application programming interface for use in __C__ applications.
See header file `can_api.h` for a description of all API functions.

#### libCANAL (_Sorry, not realized yet!_)

___libCANAL___ is a dynamic library with a VSCP CANAL compatible application programming interface.
The TouCAN USB hardware is delivered with a CANAL (CAN Application Layer) DLL for Windows.
See [Rusoku](https://github.com/rusoku)´s GitHub page for further information.

#### can_moni

`can_moni` is a command line tool to view incoming CAN messages.
I hate this messing around with binary masks for identifier filtering.
So I wrote this little program to have an exclude list for single identifiers or identifier ranges (see program option `--exclude` or just `-x`). Precede the list with a `~` and you get an include list.

Type `can_moni --help` to display all program options.

#### can_test

`can_test` is a command line tool to test CAN communication.
Originally developed for electronic environmental tests on an embedded Linux system with SocketCAN, I´m using it for many years as a traffic generator for CAN stress-tests.

Type `can_test --help` to display all program options.

### Target Platform

- macOS 11.0 and later (Intel and Apple silicon)

### Development Environment

#### macOS Monterey

- macOS Monterey (12.1) on a Mac mini (M1, 2020)
- Apple clang version 13.0.0 (clang-1300.0.29.30)
- Xcode Version 13.2.1 (13C100)

#### macOS Big Sur

- macOS Big Sur (11.6.3) on a MacBook Pro (2019)
- Apple clang version 13.0.0 (clang-1300.0.29.30)
- Xcode Version 13.2.1 (13C100)

#### macOS High Sierra

- macOS High Sierra (10.13.6) on a MacBook Pro (late 2011)
- Apple LLVM version 10.0.0 (clang-1000.11.45.5)
- Xcode Version 10.1 (10B61)

### CAN Hardware

- TouCAN USB (Model F4FS1, Hardware 1.0.0, Firmware 1.0.1)
- TouCAN USB (Model F4FS1, Hardware 1.0.0, Firmware 1.0.4)

### Testing

The Xcode project for the trial program includes a Xctest target with one test suite for each CAN API V3 **C** interface function.
To run the test suites or single test cases two CAN devices are required.
General test settings can be change in the file `Settings.h`.

## Known Bugs and Caveats

For a list of known bugs and caveats see tab [Issues](https://github.com/mac-can/RusokuCAN.dylib/issues) in the GitHub repo.

## This and That

For reasons unknown to me, the artifacts build with Xcode detect the TouCAN USB adapter only under macOS 10.15 (Catalina) and higher.
The artifacts build by Makefile also detect the adapter under macOS 10.13 (High Sierra).
So I hope that the artifacts build by Makefile are backward compatible down to version 10.8 (Mountain Lion) of the world´s mostest advanced OS.

### MacCAN-Core Repo

The MacCAN-Core sources are maintained in a SVN repo to synchronized them between the different MacCAN driver repos.

### Licenses

#### MacCAN-TouCAN License

MacCAN-TouCAN is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MacCAN-TouCAN is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MacCAN-TouCAN.  If not, see &lt;http://www.gnu.org/licenses/&gt;.

#### MacCAN-Core License

MacCAN-Core (which includes CAN API V3) is dual-licensed under the terms of the BSD 2-Clause "Simplified" License
and under the terms of the GNU General Public License v3.0 (or any later version).
The terms of the GNU General Public License v3.0 (or any later version) apply to this work, see above.

### Trademarks

Mac and macOS are trademarks of Apple Inc., registered in the U.S. and other countries. \
Windows is a registered trademark of Microsoft Corporation in the United States and/or other countries. \
Linux is a registered trademark of Linus Torvalds. \
All other company, product and service names mentioned herein are trademarks, registered trademarks or service marks of their respective owners.

### Hazard Note

_If you connect your CAN device to a real CAN network when using this library, you might damage your application._

### Contact

E-Mail: mailto://info@mac.can.com \
Internet: https://www.mac-can.net

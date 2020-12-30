### macOS&reg; User-Space Driver for TouCAN USB Interfaces from Rusoku

_Copyright &copy; 2020  Uwe Vogt, UV Software, Berlin (info@mac-can.com)_

# Running CAN on a Mac&reg;

_Running CAN on a Mac_ is the mission of the MacCAN project.
The macOS driver for TouCAN USB interfaces from [Rusoku](https://www.rusoku.com) is based on _MacCAN-Core_ which is an abstraction (or rather a wrapper) of Apple´s IOUsbKit to create USB user-space drivers for CAN interfaces from various vendors under macOS.

## MacCAN-TouCAN

This repo contains the source code for the _MacCAN-TouCAN_ driver and several alternatives to build dynamic libraries for macOS,
either as a C++ class library ([_libTouCAN_](#libTouCAN)),
or as a _CAN API V3_ driver library ([_libUVCANTOU_](#libUVCANTOU)),
or as a _VSCP CANAL_ compatible library ([_libCANAL_](#libCANAL)),
as well as some C/C++ example programs ([`can_moni`](can_moni-for-maccan-toucan) and [`can_test`](can_test-for-maccan-toucan)).


### TouCAN API

```C++
/// \name   TouCAN API
/// \brief  MacCAN driver for Rusoku TouCAN USB interfaces
/// \note   See CMacCAN for a description of the overridden methods
/// \{
class CTouCAN : public CMacCAN {
private:
    MacCAN_Handle_t m_hDevice;  ///< USB device handle
    MacCAN_OpMode_t m_OpMode;  ///< CAN operation mode
    MacCAN_Status_t m_Status;  ///< CAN status register
    MacCAN_Bitrate_t m_Bitrate;  ///< CAN bit-rate settings
    struct {
        uint64_t u64TxMessages;  ///< number of transmitted CAN messages
        uint64_t u64RxMessages;  ///< number of received CAN messages
        uint64_t u64ErrorFrames;  ///< number of received status messages
    } m_Counter;
    // opaque data type
    struct STouCAN;  ///< C++ forward declaration
    STouCAN *m_pTouCAN;  ///< TouCAN USB device interface
public:
    // constructor / destructor
    CTouCAN();
    ~CTouCAN();
    // CTouCAN-specific error codes (CAN API V3 extension)
    enum EErrorCodes {
        // note: range 0...-99 is reserved by CAN API V3
        GeneralError = VendorSpecific
    };
    // CMacCAN overrides
    static MacCAN_Return_t ProbeChannel(int32_t channel, MacCAN_OpMode_t opMode, const void *param, EChannelState &state);
    static MacCAN_Return_t ProbeChannel(int32_t channel, MacCAN_OpMode_t opMode, EChannelState &state);

    MacCAN_Return_t InitializeChannel(int32_t channel, MacCAN_OpMode_t opMode, const void *param = NULL);
    MacCAN_Return_t TeardownChannel();

    MacCAN_Return_t StartController(MacCAN_Bitrate_t bitrate);
    MacCAN_Return_t ResetController();

    MacCAN_Return_t WriteMessage(MacCAN_Message_t message, uint16_t timeout = 0U);
    MacCAN_Return_t ReadMessage(MacCAN_Message_t &message, uint16_t timeout = CANREAD_INFINITE);

    MacCAN_Return_t GetStatus(MacCAN_Status_t &status);
    MacCAN_Return_t GetBusLoad(uint8_t &load);

    MacCAN_Return_t GetBitrate(MacCAN_Bitrate_t &bitrate);
    MacCAN_Return_t GetBusSpeed(MacCAN_BusSpeed_t &speed);

    MacCAN_Return_t GetProperty(uint16_t param, void *value, uint32_t nbytes);
    MacCAN_Return_t SetProperty(uint16_t param, const void *value, uint32_t nbytes);

    char *GetHardwareVersion();  // (for compatibility reasons)
    char *GetFirmwareVersion();  // (for compatibility reasons)
    static char *GetVersion();  // (for compatibility reasons)

    // note: we have vendor-specific bit-timing (clock domain is 50MHz)
    static MacCAN_Return_t MapIndex2Bitrate(int32_t index, MacCAN_Bitrate_t &bitrate);
};
/// \}
```

### Build Targets

_Important note_: To build any of the following build targets run the `build_no.sh` script to generate a pseudo build number.
```
uv-pc013mac:~ eris$ cd ~/Projects/MacCAN/TouCAN/Sources/
uv-pc013mac:Sources eris$ ./build_no.sh
```
Repeat this step after each `git commit`, `git pull`, `git clone`, etc.

#### libTouCAN

This is a Xcode project, developed with Xcode 12.3 for macOS 11.1 (Big Sur).

___libTouCAN___ is a dynamic library with a CAN API V3 compatible application programming interface for use in __C++__ applications.
See header file `TouCAN.h` for a description of all class members.

#### libUVCANTOU

This is a `Makefile` project, so type the usual commands:
```
uv-pc013mac:Sources eris$ cd ~/Projects/MacCAN/TouCAN/Library/CANAPI/
uv-pc013mac:CANAPI eris$ make clean
uv-pc013mac:CANAPI eris$ make all
uv-pc013mac:CANAPI eris$ sudo make install
uv-pc013mac:CANAPI eris$
```
_(The version number of the library can be adapted by editing the `Makefile` and changing the variable `VERSION` accordingly.)_

___libUVCANTOU___ is a dynamic library with a CAN API V3 compatible application programming interface for use in __C__ applications.
See header file `can_api.h` for a description of all API functions.

#### libCANAL

_Sorry, not realized yet!_

___libCANAL___ is a dynamic library with a VSCP CANAL compatible application programming interface.
The TouCAN USB hardware is delivered with a CANAL (CAN Application Layer) DLL for Windows.
See [Rusoku](https://github.com/rusoku)´s GitHub page for further information.

#### can_moni for MacCAN-TouCAN

This is a Xcode project, developed with Xcode 12.3 for macOS 11.1 (Big Sur).

`can_moni` is a command line tool to view incoming CAN messages.
I hate this messing around with binary masks for identifier filtering.
So I wrote this little program to have an exclude list for single identifiers or identifier ranges (see program option `--exclude` or just `-x`). Precede the list with a `~` and you get an include list.

Type `can_moni --help` to display all program options.

#### can_test for MacCAN-TouCAN

This is a Xcode project, developed with Xcode 12.3 for macOS 11.1 (Big Sur).

`can_test` is a command line tool to test CAN communication.
Originally developed for electronic environmental tests on an embedded Linux system with SocketCAN, I´m using it for many years as a traffic generator for CAN stress-tests.

Type `can_test --help` to display all program options.

## This and That

For reasons unknown to me, the Xcode targets recognize the TouCAN USB adapter only under macOS 10.15 (Catalina). The Makefile targets also recognize the adapter under macOS 10.13 (High Sierra). So I hope that the Makefile targets are downward compatible up to version 10.6 (Mountain Lion) of the world´s mostest advanced OS.

### MacCAN-Core Repo

The MacCAN-Core sources are maintained in a SVN repo to synchronized them between the different MacCAN driver repos via Git SVN bridge.

### Licenses

#### MacCAN-Core License

MacCAN-Core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MacCAN-Core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with MacCAN-Core.  If not, see <http://www.gnu.org/licenses/>.

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
along with MacCAN-TouCAN.  If not, see <http://www.gnu.org/licenses/>.

### Trademarks

Mac and macOS are trademarks of Apple Inc., registered in the U.S. and other countries.

### Hazard Note

_If you connect your CAN device to a real CAN network when using this library, you might damage your application._

### Contact

Uwe Vogt \
UV Software \
Chausseestrasse 33a \
10115 Berlin \
Germany

E-Mail: mailto://info@mac.can.com \
Internet: https://www.mac-can.com

##### *Enjoy!*

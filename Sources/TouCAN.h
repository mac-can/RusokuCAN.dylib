//
//  TouCAN - macOS User-Space Driver for Rusoku TouCAN USB Interfaces
//
//  Copyright (C) 2020-2021  Uwe Vogt, UV Software, Berlin (info@mac-can.com)
//
//  This file is part of MacCAN-TouCAN.
//
//  MacCAN-TouCAN is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  MacCAN-TouCAN is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with MacCAN-TouCAN.  If not, see <https://www.gnu.org/licenses/>.
//
#ifndef TOUCAN_H_INCLUDED
#define TOUCAN_H_INCLUDED

#include "MacCAN.h"

/// \name   TouCAN
/// \brief  TouCAN dynamic library
/// \{
#define TOUCAN_LIBRARY_ID  CANLIB_RUSOKU_LT
#if (OPTION_CANAPI_TOUCAN_DYLIB != 0)
#define TOUCAN_LIBRARY_NAME  CANDLL_RUSOKU_LT
#else
#define TOUCAN_LIBRARY_NAME  "libTouCAN.dylib"
#endif
#define TOUCAN_LIBRARY_VENDOR  "UV Software, Berlin"
#define TOUCAN_LIBRARY_LICENSE  "GNU General Public License, Version 3"
#define TOUCAN_LIBRARY_COPYRIGHT  "Copyright (C) 2020-2021  Uwe Vogt, UV Software, Berlin"
#define TOUCAN_LIBRARY_HAZARD_NOTE  "If you connect your CAN device to a real CAN network when using this library,\n" \
                                    "you might damage your application."
/// \}

/// \name   TouCAN API
/// \brief  MacCAN driver for Rusoku TouCAN USB interfaces
/// \note   See CMacCAN for a description of the overridden methods
/// \{
class CTouCAN : public CMacCAN {
private:
    MacCAN_Handle_t m_hDevice;  ///< USB device handle
    MacCAN_OpMode_t m_OpMode;  ///< CAN operation mode
    MacCAN_Status_t m_Status;  ///< CAN status register
    MacCAN_Bitrate_t m_Bitrate;  ///< CAN bitrate settings
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
    MacCAN_Return_t SignalChannel();

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

/// \name   TouCAN Property IDs
/// \brief  Properties that can be read (or written)
/// \{
#define TOUCAN_PROPERTY_CANAPI              (CANPROP_GET_SPEC)
#define TOUCAN_PROPERTY_VERSION             (CANPROP_GET_VERSION)
#define TOUCAN_PROPERTY_PATCH_NO            (CANPROP_GET_PATCH_NO)
#define TOUCAN_PROPERTY_BUILD_NO            (CANPROP_GET_BUILD_NO)
#define TOUCAN_PROPERTY_LIBRARY_ID          (CANPROP_GET_LIBRARY_ID)
#define TOUCAN_PROPERTY_LIBRARY_NAME        (CANPROP_GET_LIBRARY_DLLNAME)
#define TOUCAN_PROPERTY_LIBRARY_VENDOR      (CANPROP_GET_LIBRARY_VENDOR)
#define TOUCAN_PROPERTY_DEVICE_TYPE         (CANPROP_GET_DEVICE_TYPE)
#define TOUCAN_PROPERTY_DEVICE_NAME         (CANPROP_GET_DEVICE_NAME)
#define TOUCAN_PROPERTY_DEVICE_VENDOR       (CANPROP_GET_DEVICE_VENDOR)
#define TOUCAN_PROPERTY_OP_CAPABILITY       (CANPROP_GET_OP_CAPABILITY)
#define TOUCAN_PROPERTY_OP_MODE             (CANPROP_GET_OP_MODE)
#define TOUCAN_PROPERTY_BITRATE             (CANPROP_GET_BITRATE)
#define TOUCAN_PROPERTY_SPEED               (CANPROP_GET_SPEED)
#define TOUCAN_PROPERTY_STATUS              (CANPROP_GET_STATUS)
#define TOUCAN_PROPERTY_BUSLOAD             (CANPROP_GET_BUSLOAD)
#define TOUCAN_PROPERTY_TX_COUNTER          (CANPROP_GET_TX_COUNTER)
#define TOUCAN_PROPERTY_RX_COUNTER          (CANPROP_GET_RX_COUNTER)
#define TOUCAN_PROPERTY_ERR_COUNTER         (CANPROP_GET_ERR_COUNTER)
#define TOUCAN_PROPERTY_CLOCK_DOMAIN        (CANPROP_GET_VENDOR_PROP + 0U)
#define TOUCAN_PROPERTY_HARDWARE_VERSION    (CANPROP_GET_VENDOR_PROP + 0x10U)
#define TOUCAN_PROPERTY_FIRMWARE_VERSION    (CANPROP_GET_VENDOR_PROP + 0x11U)
#define TOUCAN_PROPERTY_BOOTLOADER_VERSION  (CANPROP_GET_VENDOR_PROP + 0x12U)
#define TOUCAN_PROPERTY_SERIAL_NUMBER       (CANPROP_GET_VENDOR_PROP + 0x13U)
#define TOUCAN_PROPERTY_VID_PID             (CANPROP_GET_VENDOR_PROP + 0x16U)
#define TOUCAN_PROPERTY_DEVICE_ID           (CANPROP_GET_VENDOR_PROP + 0x17U)
#define TOUCAN_PROPERTY_VENDOR_URL          (CANPROP_GET_VENDOR_PROP + 0x18U)
#if (OPTION_TOUCAN_CANAL != 0)
#define TOUCAN_PROPERTY_CANAL_ERROR_STATUS  (CANPROP_GET_VENDOR_PROP + 0xF0U)  // CANAL API (r?)
#define TOUCAN_PROPERTY_CANAL_STATISTICS    (CANPROP_GET_VENDOR_PROP + 0xF1U)  // CANAL API (rw)
#endif
/// \}

#endif /* TOUCAN_H_INCLUDED */

//
//  MacCAN - macOS User-Space Driver for CAN to USB Interfaces
//
//  Copyright (C) 2012-2020  Uwe Vogt, UV Software, Berlin (info@mac-can.com)
//
//  This file is part of MacCAN-Core.
//
//  MacCAN-Core is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  MacCAN-Core is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with MacCAN-Core.  If not, see <http://www.gnu.org/licenses/>.
//
/// \file        MacCAN.cpp
///
/// \brief       MacCAN - macOS User-Space Driver for CAN to USB Interfaces
///
/// \author      $Author: eris $
///
/// \version     $Rev: 877 $
///
/// \addtogroup  mac_can
/// \{
#include "MacCAN.h"
#include "MacCAN_IOUsbKit.h"
#include "can_btr.h"

#include <stdio.h>
#include <string.h>

#if (OPTION_MACCAN_METHODS_VISIBLE != 0)
#define EXPORT  __attribute__((visibility("default")))
#else
#define EXPORT
#endif

//  Methods to initialize and release the MacCAN IOUsbKit
//
MacCAN_Return_t CMacCAN::Initializer() {
    return (MacCAN_Return_t)CANUSB_Initialize();
}

MacCAN_Return_t CMacCAN::Finalizer() {
    return (MacCAN_Return_t)CANUSB_Teardown();
}

//  Methods for bit-rate conversion
//
EXPORT
MacCAN_Return_t CMacCAN::MapIndex2Bitrate(int32_t index, MacCAN_Bitrate_t &bitrate) {
    return (MacCAN_Return_t)btr_index2bitrate(index, &bitrate);
}

EXPORT
MacCAN_Return_t CMacCAN::MapString2Bitrate(const char *string, MacCAN_Bitrate_t &bitrate) {
    bool brse = false;
    // TODO: rework function 'btr_string2bitrate'
    return (MacCAN_Return_t)btr_string2bitrate((const btr_string_t)string, &bitrate, &brse);
}

EXPORT
MacCAN_Return_t CMacCAN::MapBitrate2String(MacCAN_Bitrate_t bitrate, char *string, size_t length) {
    (void) length;
    // TODO: rework function 'btr_bitrate2string'
    return (MacCAN_Return_t)btr_bitrate2string(&bitrate, false, (btr_string_t)string);
}

EXPORT
MacCAN_Return_t CMacCAN::MapBitrate2Speed(MacCAN_Bitrate_t bitrate, MacCAN_BusSpeed_t &speed) {
    // TODO: rework function 'btr_bitrate2speed'
    return (MacCAN_Return_t)btr_bitrate2speed(&bitrate, false, false, &speed);
}

//  Method to retrieve version information about the MacCAN Core
//
EXPORT
char *CMacCAN::GetVersion() {
    static char string[CANPROP_MAX_BUFFER_SIZE] = "The torture never stops.";
    UInt32 version = CANUSB_GetVersion();
    sprintf(string, "macOS Driver Kit for CAN to USB Interfaces (MacCAN Core %u.%u.%u)",
                    (UInt8)(version >> 24), (UInt8)(version >> 16), (UInt8)(version >> 8));
    return (char *)string;
}

/// \}

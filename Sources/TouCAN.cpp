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
#include "build_no.h"
#define VERSION_MAJOR    0
#define VERSION_MINOR    2
#define VERSION_PATCH    2
#define VERSION_BUILD    BUILD_NO
#define VERSION_STRING   TOSTRING(VERSION_MAJOR) "." TOSTRING(VERSION_MINOR) "." TOSTRING(VERSION_PATCH) " (" TOSTRING(BUILD_NO) ")"
#if defined(_WIN64)
#define PLATFORM        "x64"
#elif defined(_WIN32)
#define PLATFORM        "x86"
#elif defined(__linux__)
#define PLATFORM        "Linux"
#elif defined(__APPLE__)
#define PLATFORM        "macOS"
#else
#error Unsupported architecture
#endif
static const char version[] = PLATFORM " Driver for Rusoku TouCAN USB Interfaces, Version " VERSION_STRING;

#include "TouCAN.h"
#include "can_defs.h"
#include "can_api.h"
#include "can_btr.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>

#if (OPTION_TOUCAN_DYLIB != 0)
__attribute__((constructor))
static void _initializer() {
}
__attribute__((destructor))
static void _finalizer() {
}
#define EXPORT  __attribute__((visibility("default")))
#else
#define EXPORT
#endif

#ifndef _MSC_VER
#define STRCPY_S(dest,size,src)         strcpy(dest,src)
#define STRNCPY_S(dest,size,src,len)    strncpy(dest,src,len)
#define SSCANF_S(buf,format,...)        sscanf(buf,format,__VA_ARGS__)
#define SPRINTF_S(buf,size,format,...)  sprintf(buf,format,__VA_ARGS__)
#else
#define STRCPY_S(dest,size,src)         strcpy_s(dest,size,src)
#define STRNCPY_S(dest,size,src,len)    strncpy_s(dest,size,src,len)
#define SSCANF_S(buf,format,...)        sscanf_s(buf,format,__VA_ARGS__)
#define SPRINTF_S(buf,size,format,...)  sprintf_s(buf,size,format,__VA_ARGS__)
#endif

#define TOUCAN_USB_CLOCK_DOMAIN  50000000  // FIXME: replace this

EXPORT
CTouCAN::CTouCAN() {
    m_Handle = (-1);
    m_OpMode.byte = CANMODE_DEFAULT;
    m_Bitrate.btr.frequency = TOUCAN_USB_CLOCK_DOMAIN;
    m_Bitrate.btr.nominal.brp = 10;
    m_Bitrate.btr.nominal.tseg1 = 14;
    m_Bitrate.btr.nominal.tseg2 = 5;
    m_Bitrate.btr.nominal.sjw = 4;
    m_Bitrate.btr.nominal.sam = 0;
#if (OPTION_CAN_2_0_ONLY == 0)
    m_Bitrate.btr.data.brp = m_Bitrate.btr.nominal.brp;
    m_Bitrate.btr.data.tseg1 = m_Bitrate.btr.nominal.tseg1;
    m_Bitrate.btr.data.tseg2 = m_Bitrate.btr.nominal.tseg2;
    m_Bitrate.btr.data.sjw = m_Bitrate.btr.nominal.sjw;
#endif
    m_Counter.u64TxMessages = 0U;
    m_Counter.u64RxMessages = 0U;
    m_Counter.u64ErrorFrames = 0U;
}

EXPORT
CTouCAN::~CTouCAN() {
    (void)TeardownChannel();
}

EXPORT
CANAPI_Return_t CTouCAN::ProbeChannel(int32_t channel, const CANAPI_OpMode_t &opMode, const void *param, EChannelState &state) {
    // test the CAN interface (hardware and driver)
    int result = CANBRD_NOT_TESTABLE;
    CANAPI_Return_t rc = can_test(channel, opMode.byte, param, &result);
    state = (EChannelState)result;
    return rc;
}

EXPORT
CANAPI_Return_t CTouCAN::ProbeChannel(int32_t channel, const CANAPI_OpMode_t &opMode, EChannelState &state) {
    // delegate this function call
    return ProbeChannel(channel, opMode, NULL, state);
}

EXPORT
CANAPI_Return_t CTouCAN::InitializeChannel(int32_t channel, const CANAPI_OpMode_t &opMode, const void *param) {
    // initialize the CAN interface
    CANAPI_Return_t rc = CANERR_FATAL;
    CANAPI_Handle_t hnd = can_init(channel, opMode.byte, param);
    if (0 <= hnd) {
        m_Handle = hnd;  // we got a handle
        m_OpMode = opMode;
        rc = CANERR_NOERROR;
    } else {
        rc = (CANAPI_Return_t)hnd;
    }
    return rc;
}

EXPORT
CANAPI_Return_t CTouCAN::TeardownChannel() {
    // shutdown the CAN interface
    CANAPI_Return_t rc = CANERR_HANDLE;
    if (0 <= m_Handle) {  // note: -1 will close all!
        rc = can_exit(m_Handle);
        if (CANERR_NOERROR == rc) {
            m_Handle = -1;  // invalidate the handle
        }
    }
    return rc;
}

EXPORT
CANAPI_Return_t CTouCAN::SignalChannel() {
    // signal waiting event objects of the CAN interface
    CANAPI_Return_t rc = CANERR_HANDLE;
    if (0 <= m_Handle) {  // note: -1 will kill all!
        rc = can_kill(m_Handle);
    }
    return rc;
}

EXPORT
CANAPI_Return_t CTouCAN::StartController(CANAPI_Bitrate_t bitrate) {
    // start the CAN controller with the given bit-rate settings
    CANAPI_Return_t rc = can_start(m_Handle, &bitrate);
    if (CANERR_NOERROR == rc) {
        m_Bitrate = bitrate;
        memset(&m_Counter, 0, sizeof(m_Counter));
    }
    return rc;
}

EXPORT
CANAPI_Return_t CTouCAN::ResetController() {
    // stop any operation of the CAN controller
    return can_reset(m_Handle);
}

EXPORT
CANAPI_Return_t CTouCAN::WriteMessage(CANAPI_Message_t message, uint16_t timeout) {
    // transmit a message over the CAN bus
    CANAPI_Return_t rc = can_write(m_Handle, &message, timeout);
    if (CANERR_NOERROR == rc) {
        m_Counter.u64TxMessages++;
    }
    return rc;
}

EXPORT
CANAPI_Return_t CTouCAN::ReadMessage(CANAPI_Message_t &message, uint16_t timeout) {
    // read one message from the message queue of the CAN interface, if any
    CANAPI_Return_t rc = can_read(m_Handle, &message, timeout);
    if (CANERR_NOERROR == rc) {
        m_Counter.u64RxMessages += !message.sts ? 1U : 0U;
        m_Counter.u64ErrorFrames += message.sts ? 1U : 0U;
    }
    return rc;
}

EXPORT
CANAPI_Return_t CTouCAN::GetStatus(CANAPI_Status_t &status) {
    // retrieve the status register of the CAN interface
    return can_status(m_Handle, &status.byte);
}

EXPORT
CANAPI_Return_t CTouCAN::GetBusLoad(uint8_t &load) {
    // retrieve the bus-load (in percent) of the CAN interface
    return can_busload(m_Handle, &load, NULL);
}

EXPORT
CANAPI_Return_t CTouCAN::GetBitrate(CANAPI_Bitrate_t &bitrate) {
    // retrieve the bit-rate setting of the CAN interface
    CANAPI_Return_t rc = can_bitrate(m_Handle, &bitrate, NULL);
    if (CANERR_NOERROR == rc) {
        m_Bitrate = bitrate;
    }
    return rc;
}

EXPORT
CANAPI_Return_t CTouCAN::GetBusSpeed(CANAPI_BusSpeed_t &speed) {
    // retrieve the transmission rate of the CAN interface
    return can_bitrate(m_Handle, &m_Bitrate, &speed);
}

EXPORT
CANAPI_Return_t CTouCAN::GetProperty(uint16_t param, void *value, uint32_t nbyte) {
    // backdoor access to the CAN handle (Careful with That Axe, Eugene)
    if (CANPROP_GET_CPP_BACKDOOR == param) {
        CANAPI_Return_t rc = CANERR_ILLPARA;
        if (NULL == value) {
            rc = CANERR_NULLPTR;
        }
        else if ((size_t)nbyte >= sizeof(int32_t)) {
            *(int32_t*)value = (int32_t)m_Handle;
            rc = CANERR_NOERROR;
        }
        return rc;
    }
    // retrieve a property value of the CAN interface
    return can_property(m_Handle, param, value, nbyte);
}

EXPORT
CANAPI_Return_t CTouCAN::SetProperty(uint16_t param, const void *value, uint32_t nbyte) {
    // modify a property value of the CAN interface
    return can_property(m_Handle, param, (void*)value, nbyte);
}

EXPORT
char *CTouCAN::GetHardwareVersion() {
    // retrieve the hardware version of the CAN controller
    return can_hardware(m_Handle);
}

EXPORT
char *CTouCAN::GetFirmwareVersion() {
    // retrieve the firmware version of the CAN controller
    return can_software(m_Handle);
}

EXPORT
char *CTouCAN::GetVersion() {
    return (char *)&version[0];
}

//  Methods for bit-rate conversion
//
EXPORT
CANAPI_Return_t CTouCAN::MapIndex2Bitrate(int32_t index, CANAPI_Bitrate_t &bitrate) {
    CANAPI_Return_t retVal = CCanApi::NoError;

    // note: we have vendor-specific bit-timing (CAN clock is 50MHz)
    //       so we cannot use the default bit-rate converter
    // FIXME: this work only for the TouCAN USB adapter
    switch (index) {
        case CANBTR_INDEX_1M:
            bitrate.btr.frequency = TOUCAN_USB_CLOCK_DOMAIN;
            bitrate.btr.nominal.brp = 5;
            bitrate.btr.nominal.tseg1 = 7;
            bitrate.btr.nominal.tseg2 = 2;
            bitrate.btr.nominal.sjw = 2;
            bitrate.btr.nominal.sam = 0;
            break;
        case CANBTR_INDEX_500K:
            bitrate.btr.frequency = TOUCAN_USB_CLOCK_DOMAIN;
            bitrate.btr.nominal.brp = 5;
            bitrate.btr.nominal.tseg1 = 14;
            bitrate.btr.nominal.tseg2 = 5;
            bitrate.btr.nominal.sjw = 4;
            bitrate.btr.nominal.sam = 0;
            break;
        case CANBTR_INDEX_250K:
            bitrate.btr.frequency = TOUCAN_USB_CLOCK_DOMAIN;
            bitrate.btr.nominal.brp = 10;
            bitrate.btr.nominal.tseg1 = 14;
            bitrate.btr.nominal.tseg2 = 5;
            bitrate.btr.nominal.sjw = 4;
            bitrate.btr.nominal.sam = 0;
            break;
        case CANBTR_INDEX_125K:
            bitrate.btr.frequency = TOUCAN_USB_CLOCK_DOMAIN;
            bitrate.btr.nominal.brp = 20;
            bitrate.btr.nominal.tseg1 = 14;
            bitrate.btr.nominal.tseg2 = 5;
            bitrate.btr.nominal.sjw = 4;
            bitrate.btr.nominal.sam = 0;
            break;
        case CANBTR_INDEX_100K:
            bitrate.btr.frequency = TOUCAN_USB_CLOCK_DOMAIN;
            bitrate.btr.nominal.brp = 25;
            bitrate.btr.nominal.tseg1 = 14;
            bitrate.btr.nominal.tseg2 = 5;
            bitrate.btr.nominal.sjw = 4;
            bitrate.btr.nominal.sam = 0;
            break;
        case CANBTR_INDEX_50K:
            bitrate.btr.frequency = TOUCAN_USB_CLOCK_DOMAIN;
            bitrate.btr.nominal.brp = 50;
            bitrate.btr.nominal.tseg1 = 14;
            bitrate.btr.nominal.tseg2 = 5;
            bitrate.btr.nominal.sjw = 4;
            bitrate.btr.nominal.sam = 0;
            break;
        case CANBTR_INDEX_20K:
            bitrate.btr.frequency = TOUCAN_USB_CLOCK_DOMAIN;
            bitrate.btr.nominal.brp = 125;
            bitrate.btr.nominal.tseg1 = 14;
            bitrate.btr.nominal.tseg2 = 5;
            bitrate.btr.nominal.sjw = 4;
            bitrate.btr.nominal.sam = 0;
            break;
        case CANBTR_INDEX_10K:
            bitrate.btr.frequency = TOUCAN_USB_CLOCK_DOMAIN;
            bitrate.btr.nominal.brp = 250;
            bitrate.btr.nominal.tseg1 = 14;
            bitrate.btr.nominal.tseg2 = 5;
            bitrate.btr.nominal.sjw = 4;
            bitrate.btr.nominal.sam = 0;
            break;
        default:
            retVal = CCanApi::InvalidBaudrate;
            break;
    }
#if (OPTION_CAN_2_0_ONLY == 0)
    if (retVal == CCanApi::NoError) {
        // indexes are only defined for CAN 2.0 mode!
        bitrate.btr.data.brp = bitrate.btr.nominal.brp;
        bitrate.btr.data.tseg1 = bitrate.btr.nominal.tseg1;
        bitrate.btr.data.tseg2 = bitrate.btr.nominal.tseg2;
        bitrate.btr.data.sjw = bitrate.btr.nominal.sjw;
    }
#endif
    return retVal;
}

EXPORT
CANAPI_Return_t CTouCAN::MapString2Bitrate(const char *string, CANAPI_Bitrate_t &bitrate) {
    bool brse = false;
    // TODO: rework function 'btr_string2bitrate'
    return (CANAPI_Return_t)btr_string2bitrate((btr_string_t)string, &bitrate, &brse);
}

EXPORT
CANAPI_Return_t CTouCAN::MapBitrate2String(CANAPI_Bitrate_t bitrate, char *string, size_t length) {
    (void) length;
    // TODO: rework function 'btr_bitrate2string'
    return (CANAPI_Return_t)btr_bitrate2string(&bitrate, false, (btr_string_t)string);
}

EXPORT
CANAPI_Return_t CTouCAN::MapBitrate2Speed(CANAPI_Bitrate_t bitrate, CANAPI_BusSpeed_t &speed) {
    // TODO: rework function 'btr_bitrate2speed'
    return (CANAPI_Return_t)btr_bitrate2speed(&bitrate, false, false, &speed);
}

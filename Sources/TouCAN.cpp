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
#define VERSION_PATCH    0
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
#include "TouCAN_USB.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>

#if (OPTION_TOUCAN_DYLIB != 0)
__attribute__((constructor))
static void _initializer() {
    MACCAN_DEBUG_INFO("@@@ [%s] [%s]\n", __FILE__, __FUNCTION__);
    MacCAN_Return_t retVal = CMacCAN::Initializer();
    if (retVal != CMacCAN::NoError)
        MACCAN_DEBUG_ERROR("!!! Couldn't initialize dynamic library for Rusoku TouCAN USB interfaces (%i)\n", retVal);
}
__attribute__((destructor))
static void _finalizer() {
    MACCAN_DEBUG_INFO("@@@ [%s] [%s]\n", __FILE__, __FUNCTION__);
    MacCAN_Return_t retVal = CMacCAN::Finalizer();
    if (retVal != CMacCAN::NoError)
        MACCAN_DEBUG_ERROR("!!! Couldn't finalize dynamic library for Rusoku TouCAN USB interfaces (%i)\n", retVal);
}
#define EXPORT  __attribute__((visibility("default")))
#else
#define EXPORT
#endif
#define SUPPORTED_OP_MODE  (CANMODE_DEFAULT | CANMODE_MON | CANMODE_ERR /* TODO: | CANMODE_NXTD | CANMODE_NRTR*/)
#define ASYNCHRONOUS_READ

struct CTouCAN::STouCAN {
    // attributes
    uint16_t m_u16VendorId;  ///< vendor id.
    uint16_t m_u16ProductId;  ///< product id.
    TouCAN_ReceiveData_t m_ReceiveData;  ///< CAN reception data
    TouCAN_ReceivePipe_t m_ReceivePipe;  ///< USB reception pipe
    // constructor
    STouCAN() {
        m_u16VendorId = RUSOKU_VENDOR_ID;
        m_u16ProductId = RUSOKU_TOUCAN_USB_ID;
        bzero(&m_ReceiveData, sizeof(TouCAN_ReceiveData_t));
        m_ReceivePipe = NULL;
    }
};

EXPORT
CTouCAN::CTouCAN() {
    m_hDevice = CANUSB_INVALID_HANDLE;
    m_OpMode.byte = CANMODE_DEFAULT;
    m_Status.byte = CANSTAT_RESET;
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
    // TouCAN USB device interface
    m_pTouCAN = new STouCAN();
}

EXPORT
CTouCAN::~CTouCAN() {
    (void) TeardownChannel();
}

EXPORT
MacCAN_Return_t CTouCAN::ProbeChannel(int32_t channel, MacCAN_OpMode_t opMode, const void *param, EChannelState &state) {
    MacCAN_Return_t retVal = CMacCAN::NoError;

    if (CANUSB_IsDevicePresent((CANUSB_Index_t)channel)) {
        if (CANUSB_IsDeviceOpened((CANUSB_Index_t)channel))
            state = CMacCAN::ChannelOccupied;
        else
            state = CMacCAN::ChannelAvailable;
        if ((opMode.byte & (uint8_t)(~SUPPORTED_OP_MODE)) != 0)
            retVal = CMacCAN::IllegalParameter;
    } else
        state = CMacCAN::ChannelNotAvailable;
    (void) param;
    return retVal;
}

EXPORT
MacCAN_Return_t CTouCAN::ProbeChannel(int32_t channel, MacCAN_OpMode_t opMode, EChannelState &state) {
    return ProbeChannel(channel, opMode, NULL, state);
}

EXPORT
MacCAN_Return_t CTouCAN::InitializeChannel(int32_t channel, MacCAN_OpMode_t opMode, const void *param) {
    MacCAN_Return_t retVal = CMacCAN::AlreadyInitialized;

    // (§) CAN interface must not be initialized
    if (m_hDevice == CANUSB_INVALID_HANDLE) {
        // (1) check if requested operation mode is supported
        if ((opMode.byte & (uint8_t)(~SUPPORTED_OP_MODE)) != 0) {
            MACCAN_DEBUG_ERROR("+++ TouCAN: unsupported operation mode (%02x)\n", opMode);
            retVal = CMacCAN::NotSupported;
            goto error_initialize;
        }
        // (2) open a MacCAN device (returns a device handle on success)
        m_hDevice = CANUSB_OpenDevice((CANUSB_Index_t)channel, m_pTouCAN->m_u16VendorId, m_pTouCAN->m_u16ProductId);
        if (m_hDevice == CANUSB_INVALID_HANDLE) {
            retVal = CMacCAN::NotInitialized;
            goto error_initialize;
        }
        // (3.a) create a MacCAN pipe context for reception
        m_pTouCAN->m_ReceivePipe = CANUSB_CreatePipeAsync(m_hDevice, TOUCAN_USB_RX_DATA_PIPE_REF, TOUCAN_USB_RX_DATA_PIPE_SIZE);
        if (m_pTouCAN->m_ReceivePipe == NULL) {
            (void) CANUSB_CloseDevice(m_hDevice);
            m_hDevice = CANUSB_INVALID_HANDLE;
            retVal = CMacCAN::ResourceError;
            goto error_initialize;
        }
        // (3.b) create a MacCAN message queue for reception
        m_pTouCAN->m_ReceiveData.m_MsgQueue = CANQUE_Create(TOUCAN_USB_RCV_QUEUE_SIZE, sizeof(MacCAN_Message_t));
        if (m_pTouCAN->m_ReceiveData.m_MsgQueue == NULL) {
            (void) CANUSB_DestroyPipeAsync(m_pTouCAN->m_ReceivePipe);
            (void) CANUSB_CloseDevice(m_hDevice);
            m_hDevice = CANUSB_INVALID_HANDLE;
            retVal = CMacCAN::ResourceError;
            goto error_initialize;
        }
        // (4) initialize the CAN controller
        MACCAN_DEBUG_DRIVER("    Initializing Rusoku TouCAN USB driver...\n");
        // (!) initialize the TouCAN device (reset it before)
        m_Status.byte = CANSTAT_RESET;
        retVal = TouCAN_InitializeInterface(m_hDevice);
        if (retVal != CANERR_NOERROR) {
            (void) CANQUE_Destroy(m_pTouCAN->m_ReceiveData.m_MsgQueue);
            (void) CANUSB_DestroyPipeAsync(m_pTouCAN->m_ReceivePipe);
            (void) CANUSB_CloseDevice(m_hDevice);
            m_hDevice = CANUSB_INVALID_HANDLE;
            goto error_initialize;
        }
        // (5) start the MacCAN reception loop
#ifndef SYNCHRONOUS_READ
        retVal = TouCAN_StartReception(m_pTouCAN->m_ReceivePipe, &m_pTouCAN->m_ReceiveData);
        if (retVal != CANERR_NOERROR) {
            (void) TouCAN_deinit(m_hDevice);
            (void) CANQUE_Destroy(m_pTouCAN->m_ReceiveData.m_MsgQueue);
            (void) CANUSB_DestroyPipeAsync(m_pTouCAN->m_ReceivePipe);
            (void) CANUSB_CloseDevice(m_hDevice);
            (void) CANUSB_CloseDevice(m_hDevice);
            m_hDevice = CANUSB_INVALID_HANDLE;
            goto error_initialize;
        }
#endif
        // :-) CAN controller is in INIT mode
        m_Status.can_stopped = 1;
        m_OpMode = opMode;
    }
error_initialize:
    (void) param;
    return retVal;
}

EXPORT
MacCAN_Return_t CTouCAN::TeardownChannel() {
    MacCAN_Return_t retVal = CMacCAN::NotInitialized;

    // (§) CAN interface must be initialized
    if (m_hDevice != CANUSB_INVALID_HANDLE) {
        (void) ResetController(); // m_Status.can_stopped := 1
        MACCAN_DEBUG_DRIVER("    Teardown Rusoku TouCAN USB driver...\n");
        // (5) stop the MacCAN reception loop
        retVal = TouCAN_AbortReception(m_pTouCAN->m_ReceivePipe);
        if (retVal != CANERR_NOERROR)
            goto error_teardown;
        // (!) wait a minute!
        usleep(54945U);
        // (4) de-initialize the CAN controller
        retVal = TouCAN_TeardownInterface(m_hDevice);
        if (retVal != CANERR_NOERROR)
            goto error_teardown;
        // (3.a) destroy the MacCAN pipe context for reception
        retVal = CANUSB_DestroyPipeAsync(m_pTouCAN->m_ReceivePipe);
        if (retVal != CANERR_NOERROR)
            goto error_teardown;
        // (3.b) destroy the MacCAN message queue for reception
        retVal = CANQUE_Destroy(m_pTouCAN->m_ReceiveData.m_MsgQueue);
        if (retVal != CANERR_NOERROR)
            goto error_teardown;
        // (2) close the MacCAN device
        retVal = CANUSB_CloseDevice(m_hDevice);
        if (retVal != CANERR_NOERROR)
            goto error_teardown;
        // (1) invalidate the device handle
        m_hDevice = CANUSB_INVALID_HANDLE;
    }
error_teardown:
    return retVal;
}

EXPORT
MacCAN_Return_t CTouCAN::SignalChannel() {
    MacCAN_Return_t retVal = CMacCAN::NotInitialized;

    // (§) CAN interface must be initialized
    if (m_hDevice != CANUSB_INVALID_HANDLE) {
        // TODO: signal all waitable objects
        retVal = CMacCAN::NotSupported;
    }
    return retVal;
}

EXPORT
MacCAN_Return_t CTouCAN::StartController(MacCAN_Bitrate_t bitrate) {
    MacCAN_Return_t retVal = CMacCAN::NotInitialized;
    MacCAN_Bitrate_t temporary = bitrate;

    // (§) CAN interface must be initialized
    if (m_hDevice != CANUSB_INVALID_HANDLE) {
        // (a) check bit-rate settings (possibly after conversion from index)
        if (bitrate.index <= 0) {
            // note: we have vendor-specific bit-timing (clock domain is 50MHz)
            //       the method from the base class uses the SJA1000 clock domain
            if (CTouCAN::MapIndex2Bitrate(bitrate.index, temporary) != CANERR_NOERROR)
                return CANERR_BAUDRATE;  // FIXME: single point of exit
        } else {
            if (bitrate.btr.frequency != TOUCAN_USB_CLOCK_DOMAIN)
                return CANERR_BAUDRATE;  // FIXME: single point of exit
        }
        // (§) CAN controller must be in INIT state!
        if (m_Status.can_stopped) {
            m_Status.byte = CANSTAT_RESET;
            // (1) set bit-rate and operation mode
            if ((retVal = TouCAN_SetBitrateAndMode(m_hDevice, &temporary,
                                                   &m_OpMode)) != CANERR_NOERROR)
                return retVal;  // FIXME: single point of exit
            m_Bitrate = temporary;
            // (2) clear queues, counters, time reference
            m_Counter.u64TxMessages = 0U;
            m_Counter.u64RxMessages = 0U;
            m_Counter.u64ErrorFrames = 0U;
            // TODO: pipe contex
            m_pTouCAN->m_ReceiveData.m_MsgParam.m_u8StatusByte = 0x00U;
            (void) CANQUE_Reset(m_pTouCAN->m_ReceiveData.m_MsgQueue);
            // (3) start CAN controller
            retVal = (MacCAN_Return_t)TouCAN_start(m_hDevice);
            m_Status.can_stopped = (retVal != CANERR_NOERROR) ? 1 : 0;
        } else
            retVal = CMacCAN::ControllerOnline;
    }
    return retVal;
}

EXPORT
MacCAN_Return_t CTouCAN::ResetController() {
    MacCAN_Return_t retVal = CMacCAN::NotInitialized;

    // (§) CAN interface must be initialized
    if (m_hDevice != CANUSB_INVALID_HANDLE) {
        // (§) CAN controller must not be in INIT state!
        if (!m_Status.can_stopped) {
            retVal = (MacCAN_Return_t)TouCAN_stop(m_hDevice);
            m_Status.can_stopped = (retVal == CANERR_NOERROR) ? 1 : 0;
        } else
#ifndef OPTION_CANAPI_RETVALS
            retVal = CMacCAN::ControllerOffline;
#else
            // note: CAN API `can_reset' returns CANERR_NOERROR even
            //       when the CAN controller has not been started
            retVal = CMacCAN::NoError;
#endif
    }
    return retVal;
}

EXPORT
MacCAN_Return_t CTouCAN::WriteMessage(MacCAN_Message_t message, uint16_t timeout) {
    MacCAN_Return_t retVal = CMacCAN::NotInitialized;

    // (§) CAN interface must be initialized
    if (m_hDevice != CANUSB_INVALID_HANDLE) {
        // (a) check identifier range
        if (!message.xtd && (message.id > TOUCAN_MSG_STD_MAX_ID))
            return CMacCAN::IllegalParameter;
        if (message.xtd && (message.id > TOUCAN_MSG_XTD_MAX_ID))
            return CMacCAN::IllegalParameter;
        // (b) check data length code
        if (message.dlc > TOUCAN_USB_MAX_FRAME_DLC)
            return CMacCAN::IllegalParameter;
        // (§) CAN controller must not be in INIT state!
        if (!m_Status.can_stopped) {
            UInt8 buffer[TOUCAN_USB_TX_DATA_PIPE_SIZE];
            bzero(buffer, TOUCAN_USB_TX_DATA_PIPE_SIZE);

            message.id &= (UInt32)(message.xtd ? TOUCAN_MSG_XTD_MAX_ID : TOUCAN_MSG_STD_MAX_ID);
            message.dlc = (UInt8)((message.dlc < TOUCAN_USB_MAX_FRAME_DLC) ? message.dlc : TOUCAN_USB_MAX_FRAME_DLC);

            int size = TouCAN_EncodeMessage(buffer, &message);

            retVal = CANUSB_WritePipe(m_hDevice, TOUCAN_USB_TX_DATA_PIPE_REF, buffer, (UInt32)size, timeout);
            m_Status.transmitter_busy = (retVal != CANERR_NOERROR) ? 1 : 0;
            m_Counter.u64TxMessages += (retVal == CANERR_NOERROR) ? 1U : 0U;

            // TODO: implement a transmit queue to speed up sending
            (void) timeout;
        } else
            retVal = CMacCAN::ControllerOffline;
    }
    return retVal;
}

EXPORT
MacCAN_Return_t CTouCAN::ReadMessage(MacCAN_Message_t &message, uint16_t timeout) {
    MacCAN_Return_t retVal = CMacCAN::NotInitialized;

    // (§) CAN interface must be initialized
    if (m_hDevice != CANUSB_INVALID_HANDLE) {
        // (§) CAN controller must not be in INIT state!
        if (!m_Status.can_stopped) {
#ifdef SYNCHRONOUS_READ
            MacCAN_Message_t frame;
            UInt8 buffer[TOUCAN_USB_RX_DATA_PIPE_SIZE];
            bzero(buffer, TOUCAN_USB_RX_DATA_PIPE_SIZE);
            UInt32 size = TOUCAN_USB_RX_DATA_PIPE_SIZE;
            UInt32 index = 0U;
            (void) timeout;

            retVal = CANUSB_ReadPipe(m_hDevice, TOUCAN_USB_RX_DATA_PIPE_REF, (void *)buffer, &size);
            if (retVal == 0) {
                printf("{%u} ",size);fflush(stdout);
                while (size >= TOUCAN_USB_RX_DATA_FRAME_SIZE) {
                    (void) TouCAN_DecodeMessage(&frame, &buffer[index]);
                    (void) CANQUE_Enqueue(m_pTouCAN->m_ReceiveData.m_MsgQueue, &frame);
                    index += TOUCAN_USB_RX_DATA_FRAME_SIZE;
                    size -= TOUCAN_USB_RX_DATA_FRAME_SIZE;
                }
                retVal = CANQUE_Dequeue(m_pTouCAN->m_ReceiveData.m_MsgQueue, &message, 0);
            }
            else printf("[%x]\n",retVal);
#else
            retVal = CANQUE_Dequeue(m_pTouCAN->m_ReceiveData.m_MsgQueue, &message, timeout);
#endif
            m_Status.receiver_empty = (retVal != CANERR_NOERROR) ? 1 : 0;
            m_Status.queue_overrun = CANQUE_OverflowFlag(m_pTouCAN->m_ReceiveData.m_MsgQueue) ? 1 : 0;
            m_Counter.u64RxMessages += ((retVal == CANERR_NOERROR) && !message.sts) ? 1U : 0U;
            m_Counter.u64ErrorFrames += ((retVal == CANERR_NOERROR) && message.sts) ? 1U : 0U;
        } else
            retVal = CMacCAN::ControllerOffline;
    }
    return retVal;
}

EXPORT
MacCAN_Return_t CTouCAN::GetStatus(MacCAN_Status_t &status) {
    MacCAN_Return_t retVal = CMacCAN::NotInitialized;

    // (§) CAN interface must be initialized
    if (m_hDevice != CANUSB_INVALID_HANDLE) {
        // (1) get status from received status frames
        m_Status.byte |= m_pTouCAN->m_ReceiveData.m_MsgParam.m_u8StatusByte;
        // (2) get (and clear) interface error code
        UInt32 errorCode;
        retVal = TouCAN_get_interface_error_code(m_hDevice, &errorCode);
        if (retVal == CANERR_NOERROR) {
            // FIXME: error code is always 0 even when there are errors on the bus
#if (0)
            m_Status.warning_level = (errorCode & (HAL_CAN_ERROR_EWG |
                                                   HAL_CAN_ERROR_EPV)) ? 1 : 0;
            m_Status.bus_off = (errorCode & HAL_CAN_ERROR_BOF) ? 1 : 0;
            m_Status.bus_error = (errorCode & (HAL_CAN_ERROR_STF |
                                               HAL_CAN_ERROR_FOR |
                                               HAL_CAN_ERROR_ACK |
                                               HAL_CAN_ERROR_BR  |
                                               HAL_CAN_ERROR_BD  |
                                               HAL_CAN_ERROR_CRC)) ? 1 : 0;
            m_Status.message_lost = (errorCode & (HAL_CAN_ERROR_RX_FOV0 |
                                                  HAL_CAN_ERROR_RX_FOV1)) ? 1 : 0;
            m_Status.transmitter_busy = (errorCode & (HAL_CAN_ERROR_TX_ALST0 |
                                                      HAL_CAN_ERROR_TX_TERR0 |
                                                      HAL_CAN_ERROR_TX_ALST1 |
                                                      HAL_CAN_ERROR_TX_TERR1 |
                                                      HAL_CAN_ERROR_TX_ALST2 |
                                                      HAL_CAN_ERROR_TX_TERR2)) ? 1 : 0;
#endif
        }
        (void) TouCAN_clear_interface_error_code(m_hDevice);
        // return updated status register
        status = m_Status;
    }
    return retVal;
}

EXPORT
MacCAN_Return_t CTouCAN::GetBusLoad(uint8_t &load) {
    MacCAN_Return_t retVal = CMacCAN::NotInitialized;

    // (§) CAN interface must be initialized
    if (m_hDevice != CANUSB_INVALID_HANDLE) {
        // TODO: get bus load
        load = 0;
        retVal = CMacCAN::NotSupported;
    }
    return retVal;
}

EXPORT
MacCAN_Return_t CTouCAN::GetBitrate(MacCAN_Bitrate_t &bitrate) {
    MacCAN_Return_t retVal = CMacCAN::NotInitialized;

    // (§) CAN interface must be initialized
    if (m_hDevice != CANUSB_INVALID_HANDLE) {
        MacCAN_Bitrate_t temporary = m_Bitrate;
        if (m_Bitrate.index <= 0) {
            // note: we have vendor-specific bit-timing (clock domain is 50MHz)
            //       the method from the base class uses the SJA1000 clock domain
            if (CTouCAN::MapIndex2Bitrate(m_Bitrate.index, temporary) != CANERR_NOERROR)
                return CANERR_BAUDRATE;  // FIXME: single point of exit
        }
        bitrate = temporary;
#ifndef OPTION_CANAPI_RETVALS
        retVal = CMacCAN::NoError;
#else
        // note: CAN API `can_bitrate' returns CANERR_OFFLINE
        //       when the CAN controller has not been started
        if (m_Status.can_stopped)
            retVal = CMacCAN::ControllerOffline;
        else
            retVal = CMacCAN::NoError;
#endif
    }
    return retVal;
}

EXPORT
MacCAN_Return_t CTouCAN::GetBusSpeed(MacCAN_BusSpeed_t &speed) {
    MacCAN_Return_t retVal = CMacCAN::NotInitialized;

    // (§) CAN interface must be initialized
    if (m_hDevice != CANUSB_INVALID_HANDLE) {
        MacCAN_Bitrate_t temporary;
        retVal = GetBitrate(temporary);
#ifndef OPTION_CANAPI_RETVALS
        if (retVal == CMacCAN::NoError) {
#else
        // note: CAN API `can_bitrate' returns CANERR_OFFLINE
        //       when the CAN controller has not been started
        if ((retVal == CMacCAN::NoError) ||
            (retVal == CMacCAN::ControllerOffline)) {
#endif
            // note: we got bit-rate settings, not an index, so
            //       we can use the method from the base class
            if (CMacCAN::MapBitrate2Speed(temporary, speed) != CANERR_NOERROR)
                return CANERR_BAUDRATE;  // FIXME: single point of exit
        }
    }
    return retVal;
}

EXPORT
MacCAN_Return_t CTouCAN::GetProperty(uint16_t param, void *value, uint32_t nbytes) {
    MacCAN_Return_t retVal = CMacCAN::IllegalParameter;

    if (!value)
        return CMacCAN::NullPointer;

    switch (param) {
        case TOUCAN_PROPERTY_CANAPI:
            if ((size_t)nbytes == sizeof(uint16_t)) {
                *(uint16_t*)value = (uint16_t)CAN_API_SPEC;
                retVal = CMacCAN::NoError;
            }
            break;
        case TOUCAN_PROPERTY_VERSION:
            if ((size_t)nbytes == sizeof(uint16_t)) {
                *(uint16_t*)value = ((uint16_t)VERSION_MAJOR << 8) | (uint16_t)VERSION_MINOR;
                retVal = CMacCAN::NoError;
            }
            break;
        case TOUCAN_PROPERTY_PATCH_NO:
            if ((size_t)nbytes == sizeof(uint8_t)) {
                *(uint8_t*)value = (uint8_t)VERSION_PATCH;
                retVal = CMacCAN::NoError;
            }
            break;
        case TOUCAN_PROPERTY_BUILD_NO:
            if ((size_t)nbytes == sizeof(uint32_t)) {
                *(uint32_t*)value = (uint32_t)VERSION_BUILD;
                retVal = CMacCAN::NoError;
            }
            break;
        case TOUCAN_PROPERTY_LIBRARY_ID:
            if ((size_t)nbytes == sizeof(int32_t)) {
                *(int32_t*)value = (int32_t)TOUCAN_LIBRARY_ID;
                retVal = CMacCAN::NoError;
            }
            break;
        case TOUCAN_PROPERTY_LIBRARY_VENDOR:
            if ((nbytes > strlen(TOUCAN_LIBRARY_VENDOR)) && (nbytes <= CANPROP_MAX_BUFFER_SIZE)) {
                strcpy((char*)value, TOUCAN_LIBRARY_VENDOR);
                retVal = CMacCAN::NoError;
            }
            break;
        case TOUCAN_PROPERTY_LIBRARY_NAME:
            if ((nbytes > strlen(TOUCAN_LIBRARY_NAME)) && (nbytes <= CANPROP_MAX_BUFFER_SIZE)) {
                strcpy((char*)value, TOUCAN_LIBRARY_NAME);
                retVal = CMacCAN::NoError;
            }
            break;
        default:
            // (§) CAN interface must be initialized to get these properties
            if (m_hDevice != CANUSB_INVALID_HANDLE) {
                switch (param) {
                    case TOUCAN_PROPERTY_DEVICE_TYPE:
                        if ((size_t)nbytes == sizeof(int32_t)) {
                            *(int32_t*)value = (int32_t)TOUCAN_USB_DEVICE_TYPE;
                            retVal = CMacCAN::NoError;
                        }
                        break;
                    case TOUCAN_PROPERTY_DEVICE_NAME:
                        if ((nbytes > strlen(TOUCAN_USB_DEVICE_NAME)) && (nbytes <= CANPROP_MAX_BUFFER_SIZE)) {
                            strcpy((char*)value, TOUCAN_USB_DEVICE_NAME);
                            retVal = CMacCAN::NoError;
                        }
                        break;
                    case TOUCAN_PROPERTY_DEVICE_VENDOR:
                        if ((nbytes > strlen(TOUCAN_USB_VENDOR_NAME)) && (nbytes <= CANPROP_MAX_BUFFER_SIZE)) {
                            strcpy((char*)value, TOUCAN_USB_VENDOR_NAME);
                            retVal = CMacCAN::NoError;
                        }
                        break;
                    case TOUCAN_PROPERTY_OP_CAPABILITY:
                        if ((size_t)nbytes == sizeof(uint8_t)) {
                            *(uint8_t*)value = (uint8_t)SUPPORTED_OP_MODE;
                            retVal = CMacCAN::NoError;
                        }
                        break;
                    case TOUCAN_PROPERTY_OP_MODE:
                        if ((size_t)nbytes == sizeof(uint8_t)) {
                            *(uint8_t*)value = (uint8_t)m_OpMode.byte;
                            retVal = CMacCAN::NoError;
                        }
                        break;
                    case TOUCAN_PROPERTY_BITRATE:
                        MacCAN_Bitrate_t bitrate;
                        if ((retVal = GetBitrate(bitrate)) == CANERR_NOERROR) {
                            if (nbytes == sizeof(MacCAN_Bitrate_t)) {
                                memcpy(value, &bitrate, sizeof(MacCAN_Bitrate_t));
                                retVal = CMacCAN::NoError;
                            }
                        }
                        break;
                    case TOUCAN_PROPERTY_SPEED:
                        MacCAN_BusSpeed_t speed;
                        if ((retVal = GetBusSpeed(speed)) == CANERR_NOERROR) {
                            if (nbytes == sizeof(MacCAN_BusSpeed_t)) {
                                memcpy(value, &speed, sizeof(MacCAN_BusSpeed_t));
                                retVal = CMacCAN::NoError;
                            }
                        }
                        break;
                    case TOUCAN_PROPERTY_STATUS:
                        MacCAN_Status_t status;
                        if ((retVal = GetStatus(status)) == CANERR_NOERROR) {
                            if ((size_t)nbytes == sizeof(uint8_t)) {
                                *(uint8_t*)value = (uint8_t)status.byte;
                                retVal = CMacCAN::NoError;
                            }
                        }
                        break;
                    case TOUCAN_PROPERTY_BUSLOAD:
                        uint8_t load;
                        if ((retVal = GetBusLoad(load)) == CANERR_NOERROR) {
                            if ((size_t)nbytes == sizeof(uint8_t)) {
                                *(uint8_t*)value = (uint8_t)load;
                                retVal = CMacCAN::NoError;
                            }
                        }
                        break;
                    case TOUCAN_PROPERTY_TX_COUNTER:
                        if ((size_t)nbytes == sizeof(uint64_t)) {
                            *(uint64_t*)value = (uint64_t)m_Counter.u64TxMessages;
                            retVal = CMacCAN::NoError;
                        }
                        break;
                    case TOUCAN_PROPERTY_RX_COUNTER:
                        if ((size_t)nbytes == sizeof(uint64_t)) {
                            *(uint64_t*)value = (uint64_t)m_Counter.u64RxMessages;
                            retVal = CMacCAN::NoError;
                        }
                        break;
                    case TOUCAN_PROPERTY_ERR_COUNTER:
                        if ((size_t)nbytes == sizeof(uint64_t)) {
                            *(uint64_t*)value = (uint64_t)m_Counter.u64ErrorFrames;
                            retVal = CMacCAN::NoError;
                        }
                        break;
                    case TOUCAN_PROPERTY_CLOCK_DOMAIN:
                        if ((size_t)nbytes == sizeof(int32_t)) {
                            *(int32_t*)value = (int32_t)TOUCAN_USB_CLOCK_DOMAIN;
                            retVal = CMacCAN::NoError;
                        }
                        break;
                    case TOUCAN_PROPERTY_HARDWARE_VERSION:
                        uint32_t hardware;
                        if ((retVal = TouCAN_get_hardware_version(m_hDevice, &hardware)) == CANERR_NOERROR) {
                            if ((size_t)nbytes == sizeof(uint32_t)) {
                                *(uint32_t*)value = (uint32_t)hardware;
                                retVal = CMacCAN::NoError;
                            }
                        }
                        break;
                    case TOUCAN_PROPERTY_FIRMWARE_VERSION:
                        uint32_t firmware;
                        if ((retVal = TouCAN_get_firmware_version(m_hDevice, &firmware)) == CANERR_NOERROR) {
                            if ((size_t)nbytes == sizeof(uint32_t)) {
                                *(uint32_t*)value = (uint32_t)firmware;
                                retVal = CMacCAN::NoError;
                            }
                        }
                        break;
                    case TOUCAN_PROPERTY_BOOTLOADER_VERSION:
                        uint32_t bootloader;
                        if ((retVal = TouCAN_get_bootloader_version(m_hDevice, &bootloader)) == CANERR_NOERROR) {
                            if ((size_t)nbytes == sizeof(uint32_t)) {
                                *(uint32_t*)value = (uint32_t)bootloader;
                                retVal = CMacCAN::NoError;
                            }
                        }
                        break;
                    case TOUCAN_PROPERTY_SERIAL_NUMBER:
                        uint32_t serial_no;
                        if ((retVal = TouCAN_get_serial_number(m_hDevice, &serial_no)) == CANERR_NOERROR) {
                            if ((size_t)nbytes == sizeof(uint32_t)) {
                                *(uint32_t*)value = (uint32_t)serial_no;
                                retVal = CMacCAN::NoError;
                            }
                        }
                        break;
                    case TOUCAN_PROPERTY_VID_PID:
                        uint32_t vid_pid;
                        if ((retVal = TouCAN_get_vid_pid(m_hDevice, &vid_pid)) == CANERR_NOERROR) {
                            if ((size_t)nbytes == sizeof(uint32_t)) {
                                *(uint32_t*)value = (uint32_t)vid_pid;
                                retVal = CMacCAN::NoError;
                            }
                        }
                        break;
                    case TOUCAN_PROPERTY_DEVICE_ID:
                        uint32_t device_id;
                        if ((retVal = TouCAN_get_device_id(m_hDevice, &device_id)) == CANERR_NOERROR) {
                            if ((size_t)nbytes == sizeof(uint32_t)) {
                                *(uint32_t*)value = (uint32_t)device_id;
                                retVal = CMacCAN::NoError;
                            }
                        }
                        break;
                    case TOUCAN_PROPERTY_VENDOR_URL:
                        retVal = TouCAN_get_vendor(m_hDevice, (unsigned int)nbytes, (char *)value);
                        break;
                    default:
                        retVal = CMacCAN::NotSupported;
                        break;
                }
            } else {
                retVal = CMacCAN::NotInitialized;
            }
            break;
    }
    return retVal;
}

EXPORT
MacCAN_Return_t CTouCAN::SetProperty(uint16_t param, const void *value, uint32_t nbytes) {
    MacCAN_Return_t retVal = CMacCAN::IllegalParameter;

    if (!value)
        return CMacCAN::NullPointer;

    // TODO: insert coin here
    (void) param;
    (void) nbytes;

    return retVal;
}

EXPORT
char *CTouCAN::GetHardwareVersion() {
    static char hardware[CANPROP_MAX_BUFFER_SIZE] = "";

    // (§) CAN interface must be initialized
    if (m_hDevice != CANUSB_INVALID_HANDLE) {
        uint32_t version;
        if (TouCAN_get_hardware_version(m_hDevice, &version) == CANERR_NOERROR) {
            sprintf(hardware, TOUCAN_USB_DEVICE_NAME ", Hardware %u.%u.%u",
                    (uint8_t)(version >> 24), (uint8_t)(version >> 16), (uint8_t)(version >> 8));
            return hardware;
        }
    }
    return NULL;
}

EXPORT
char *CTouCAN::GetFirmwareVersion() {
    static char firmware[CANPROP_MAX_BUFFER_SIZE] = "";

    // (§) CAN interface must be initialized
    if (m_hDevice != CANUSB_INVALID_HANDLE) {
        uint32_t version;
        if (TouCAN_get_firmware_version(m_hDevice, &version) == CANERR_NOERROR) {
            sprintf(firmware, TOUCAN_USB_DEVICE_NAME ", Firmware %u.%u.%u",
                    (uint8_t)(version >> 24), (uint8_t)(version >> 16), (uint8_t)(version >> 8));
            return firmware;
        }
    }
    return NULL;
}

EXPORT
char *CTouCAN::GetVersion() {
    return (char *)&version[0];
}

EXPORT
MacCAN_Return_t CTouCAN::MapIndex2Bitrate(int32_t index, MacCAN_Bitrate_t &bitrate) {
    MacCAN_Return_t retVal = CMacCAN::NoError;

    // note: we have vendor-specific bit-timing (clock domain is 50MHz)
    //       so override the method from the base class
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
            retVal = CMacCAN::InvalidBaudrate;
            break;
    }
#if (OPTION_CAN_2_0_ONLY == 0)
    if (retVal == CMacCAN::NoError) {
        // indexes are only defined for CAN 2.0 mode!
        bitrate.btr.data.brp = bitrate.btr.nominal.brp;
        bitrate.btr.data.tseg1 = bitrate.btr.nominal.tseg1;
        bitrate.btr.data.tseg2 = bitrate.btr.nominal.tseg2;
        bitrate.btr.data.sjw = bitrate.btr.nominal.sjw;
    }
#endif
    return retVal;
}

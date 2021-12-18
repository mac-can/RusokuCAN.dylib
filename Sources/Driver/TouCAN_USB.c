/*
 * TouCAN - macOS User-Space Driver for Rusoku TouCAN USB Adapters
 *
 * Copyright (C) 2020-2021  Uwe Vogt, UV Software, Berlin (info@mac-can.com)
 *
 * The following source code was copied from Rusoku's CANAL-DLL for Windows
 * and modified to run under macOS 10.13 and later.
 *
 * source https://github.com/rusoku/CANAL-DLL/blob/master/CTouCANobjCmdMsg.cpp
 * commit 0e7510019561157ac2ee4199ddbf39652b146451
 */
/*
 * CANAL interface DLL for RUSOKU technologies for TouCAN, TouCAN Marine, TouCAN Duo USB to CAN bus converter
 *
 * Copyright (C) 2000-2008 Ake Hedman, eurosource, <akhe@eurosource.se>
 * Copyright (C) 2020 Gediminas Simanskis (gediminas@rusoku.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; version 3.0 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.
 *
 */
#include "TouCAN_USB.h"

#include "MacCAN_IOUsbKit.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

//////////////////////////////////////////////////////////////////////
// TouCAN init
//

int TouCAN_init(CANUSB_Handle_t handle, UInt16 brp, UInt8 tseg1, UInt8 tseg2, UInt8 sjw, UInt32 flags) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 res;
    UInt8 data[9];
    bzero(data, 9);

    SetupPacket.RequestType = USB_HOST_TO_DEVICE | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_CAN_INTERFACE_INIT;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 9;
    
    // tseg1
    data[0] = tseg1;
    // tseg2
    data[1] = tseg2;
    // sjw
    data[2] = sjw;
    // Brp

    data[3] = (UInt8) ((brp >> 8) & 0xFF);
    data[4] = (UInt8) (brp & 0xFF);

    // flags
    data[5] = (UInt8) ((flags >> 24) & 0xFF);
    data[6] = (UInt8) ((flags >> 16) & 0xFF);
    data[7] = (UInt8) ((flags >> 8) & 0xFF);
    data[8] = (UInt8)  (flags & 0xFF);

    // TouCAN_CAN_INTERFACE_INIT
    retVal = CANUSB_DeviceRequest(handle, SetupPacket, (void *)data, 9, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(handle, &res);
    if (retVal < 0)
        return (int)retVal;
    
    if (res != TouCAN_RETVAL_OK)
        return (int)TOUCAN_ERROR_OFFSET - (int)res;
    
    return (int)TOUCAN_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// TouCAN deinit
//

int TouCAN_deinit(CANUSB_Handle_t handle) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 res;

    SetupPacket.RequestType = USB_HOST_TO_DEVICE | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_CAN_INTERFACE_DEINIT;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 0;
    
    // TouCAN_CAN_INTERFACE_DEINIT
    retVal = CANUSB_DeviceRequest(handle, SetupPacket, NULL, 0, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(handle, &res);
    if (retVal < 0)
        return (int)retVal;
    
    if (res != TouCAN_RETVAL_OK)
        return (int)TOUCAN_ERROR_OFFSET - (int)res;
    
    return (int)TOUCAN_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// TouCAN start
//

int TouCAN_start(CANUSB_Handle_t handle) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 res;

    SetupPacket.RequestType = USB_HOST_TO_DEVICE | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_CAN_INTERFACE_START;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 0;
    
    // TouCAN_CAN_INTERFACE_START
    retVal = CANUSB_DeviceRequest(handle, SetupPacket, NULL, 0, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(handle, &res);
    if (retVal < 0)
        return (int)retVal;
    
    if (res != TouCAN_RETVAL_OK)
        return (int)TOUCAN_ERROR_OFFSET - (int)res;
    
    return (int)TOUCAN_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// TouCAN stop
//

int TouCAN_stop(CANUSB_Handle_t handle) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 res;

    SetupPacket.RequestType = USB_HOST_TO_DEVICE | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_CAN_INTERFACE_STOP;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 0;

    // TouCAN_CAN_INTERFACE_STOP
    retVal = CANUSB_DeviceRequest(handle, SetupPacket, NULL, 0, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(handle, &res);
    if (retVal < 0)
        return (int)retVal;
    
    if (res != TouCAN_RETVAL_OK)
        return (int)TOUCAN_ERROR_OFFSET - (int)res;
    
    return (int)TOUCAN_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// TouCAN HAL get last error
//

int TouCAN_get_last_error_code(CANUSB_Handle_t handle, UInt8 *res) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt32 wLenDone;
    UInt8 LastErrorCode;

    SetupPacket.RequestType = USB_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_GET_LAST_ERROR_CODE;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 1;

    retVal = CANUSB_DeviceRequest(handle, SetupPacket, (void *)&LastErrorCode, 1, &wLenDone);
    if (retVal < 0)
        return (int)retVal;
    
    if ((wLenDone != 1) && (LastErrorCode != HAL_OK))  // FIXME: OR?
        return (int)TOUCAN_ERROR_OFFSET;
    
    if (res)
        *res = LastErrorCode;
    
    return (int)TOUCAN_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////
// TouCAN get CAN interface ERROR:  hcan->ErrorCode;
//

int TouCAN_get_interface_error_code(CANUSB_Handle_t handle, UInt32 *ErrorCode) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 buf[4];
    UInt8 res;

    SetupPacket.RequestType = USB_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_GET_CAN_INTERFACE_ERROR_CODE;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 4;

    retVal = CANUSB_DeviceRequest(handle, SetupPacket, (void *)buf, 4, NULL);
    if (retVal < 0)
        return (int)retVal;

    retVal = TouCAN_get_last_error_code(handle, &res);
    if (retVal < 0)
        return (int)retVal;
    
    if (res != TouCAN_RETVAL_OK)
        return (int)TOUCAN_ERROR_OFFSET - (int)res;
    
    if (ErrorCode) {
        *ErrorCode = (UInt32)buf[0] << 24;
        *ErrorCode |= (UInt32)buf[1] << 16;
        *ErrorCode |= (UInt32)buf[2] << 8;
        *ErrorCode |= (UInt32)buf[3];
    }
    return (int)TOUCAN_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////
// TouCAN TouCAN_CLEAR_CAN_INTERFACE_ERROR_CODE:  hcan->ErrorCode;
//

int TouCAN_clear_interface_error_code(CANUSB_Handle_t handle) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 res;

    SetupPacket.RequestType = USB_HOST_TO_DEVICE | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_CLEAR_CAN_INTERFACE_ERROR_CODE;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 0;

    retVal = CANUSB_DeviceRequest(handle, SetupPacket, NULL, 0, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(handle, &res);
    if (retVal < 0)
        return (int)retVal;
    
    if (res != TouCAN_RETVAL_OK)
        return (int)TOUCAN_ERROR_OFFSET - (int)res;
    
    return (int)TOUCAN_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// TouCAN TouCAN_GET_CAN_INTERFACE_STATE   hcan->State;
//

int TouCAN_get_interface_state(CANUSB_Handle_t handle, UInt8 *state) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 buf[1];
    UInt8 res;

    SetupPacket.RequestType = USB_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_GET_CAN_INTERFACE_STATE;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 1;

    retVal = CANUSB_DeviceRequest(handle, SetupPacket, (void *)buf, 1, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(handle, &res);
    if (retVal < 0)
        return (int)retVal;
    
    if (res != TouCAN_RETVAL_OK)
        return (int)TOUCAN_ERROR_OFFSET - (int)res;
    
    if (state) {
        *state = buf[0];
    }
    return (int)TOUCAN_ERROR_SUCCESS;
}

#if (OPTION_TOUCAN_CANAL != 0)
//////////////////////////////////////////////////////////////////////
// TouCAN TouCAN_get_statistics VSCP
//

int TouCAN_get_statistics(CANUSB_Handle_t handle, PCANALSTATISTICS statistics) {
    return (int)TOUCAN_ERROR_OFFSET;
}

//////////////////////////////////////////////////////////////////////
// TouCAN_clear_statistics VSCP
//

int TouCAN_clear_statistics(CANUSB_Handle_t handle) {
    return (int)TOUCAN_ERROR_OFFSET;
}

//////////////////////////////////////////////////////////////////////
// TouCAN_get_status VSCP
//

int TouCAN_get_canal_status(CANUSB_Handle_t handle, canalStatus *status) {
    return (int)TOUCAN_ERROR_OFFSET;
}
#endif

//////////////////////////////////////////////////////////////////////
// TouCAN_get_hardware_version
//
 
int TouCAN_get_hardware_version(CANUSB_Handle_t handle, UInt32 *ver) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 buf[4];
    UInt8 res;

    SetupPacket.RequestType = USB_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_GET_HARDWARE_VERSION;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 4;

    retVal = CANUSB_DeviceRequest(handle, SetupPacket, (void *)buf, 4, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(handle, &res);
    if (retVal < 0)
        return (int)retVal;
    
    if (res != TouCAN_RETVAL_OK)
        return (int)TOUCAN_ERROR_OFFSET - (int)res;
    
    if (ver) {
        *ver = (UInt32)buf[0] << 24;
        *ver |= (UInt32)buf[1] << 16;
        *ver |= (UInt32)buf[2] << 8;
        *ver |= (UInt32)buf[3];
    }
    return retVal;
}

//////////////////////////////////////////////////////////////////////
// TouCAN_get_firmware_version
//
//
int TouCAN_get_firmware_version(CANUSB_Handle_t handle, UInt32 *ver) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 buf[4];
    UInt8 res;

    SetupPacket.RequestType = USB_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_GET_FIRMWARE_VERSION;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 4;

    retVal = CANUSB_DeviceRequest(handle, SetupPacket, (void *)buf, 4, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(handle, &res);
    if (retVal < 0)
        return (int)retVal;
    
    if (res != TouCAN_RETVAL_OK)
        return (int)TOUCAN_ERROR_OFFSET - (int)res;
    
    if (ver) {
        *ver = (UInt32)buf[0] << 24;
        *ver |= (UInt32)buf[1] << 16;
        *ver |= (UInt32)buf[2] << 8;
        *ver |= (UInt32)buf[3];
    }
    return retVal;
}

//////////////////////////////////////////////////////////////////////
// TouCAN_get_bootloader_version
//
//
int TouCAN_get_bootloader_version(CANUSB_Handle_t handle, UInt32 *ver) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 buf[4];
    UInt8 res;

    SetupPacket.RequestType = USB_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_GET_BOOTLOADER_VERSION;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 4;

    retVal = CANUSB_DeviceRequest(handle, SetupPacket, (void *)buf, 4, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(handle, &res);
    if (retVal < 0)
        return (int)retVal;
    
    if (res != TouCAN_RETVAL_OK)
        return (int)TOUCAN_ERROR_OFFSET - (int)res;
    
    if (ver) {
        *ver = (UInt32)buf[0] << 24;
        *ver |= (UInt32)buf[1] << 16;
        *ver |= (UInt32)buf[2] << 8;
        *ver |= (UInt32)buf[3];
    }
    return retVal;
}

//////////////////////////////////////////////////////////////////////
// TouCAN_get_serial_number
//
//
int TouCAN_get_serial_number(CANUSB_Handle_t handle, UInt32 *ver) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 buf[4];
    UInt8 res;

    SetupPacket.RequestType = USB_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_GET_SERIAL_NUMBER;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 4;

    retVal = CANUSB_DeviceRequest(handle, SetupPacket, (void *)buf, 4, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(handle, &res);
    if (retVal < 0)
        return (int)retVal;
    
    if (res != TouCAN_RETVAL_OK)
        return (int)TOUCAN_ERROR_OFFSET - (int)res;
    
    if (ver) {
        *ver = (UInt32)buf[0] << 24;
        *ver |= (UInt32)buf[1] << 16;
        *ver |= (UInt32)buf[2] << 8;
        *ver |= (UInt32)buf[3];
    }
    return retVal;

}

//////////////////////////////////////////////////////////////////////
// TouCAN_get_vid_pid
//
//
int TouCAN_get_vid_pid(CANUSB_Handle_t handle, UInt32 *ver) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 buf[4];
    UInt8 res;

    SetupPacket.RequestType = USB_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_GET_VID_PID;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 4;

    retVal = CANUSB_DeviceRequest(handle, SetupPacket, (void *)buf, 4, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(handle, &res);
    if (retVal < 0)
        return (int)retVal;
    
    if (res != TouCAN_RETVAL_OK)
        return (int)TOUCAN_ERROR_OFFSET - (int)res;
    
    if (ver) {
        *ver = (UInt32)buf[0] << 24;
        *ver |= (UInt32)buf[1] << 16;
        *ver |= (UInt32)buf[2] << 8;
        *ver |= (UInt32)buf[3];
    }
    return retVal;
}

//////////////////////////////////////////////////////////////////////
// TouCAN_get_device_id
//
//
int TouCAN_get_device_id(CANUSB_Handle_t handle, UInt32 *ver) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 buf[4];
    UInt8 res;

    SetupPacket.RequestType = USB_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_GET_DEVICE_ID;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 4;

    retVal = CANUSB_DeviceRequest(handle, SetupPacket, (void *)buf, 4, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(handle, &res);
    if (retVal < 0)
        return (int)retVal;
    
    if (res != TouCAN_RETVAL_OK)
        return (int)TOUCAN_ERROR_OFFSET - (int)res;
    
    if (ver) {
        *ver = (UInt32)buf[0] << 24;
        *ver |= (UInt32)buf[1] << 16;
        *ver |= (UInt32)buf[2] << 8;
        *ver |= (UInt32)buf[3];
    }
    return retVal;
}

//////////////////////////////////////////////////////////////////////
// TouCAN_get_vendor
//
//
int TouCAN_get_vendor(CANUSB_Handle_t handle, unsigned int size, char *str) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 buf[32];
    UInt8 res;

    SetupPacket.RequestType = USB_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_GET_VENDOR;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 32;  // FIXME: 4(?)

    retVal = CANUSB_DeviceRequest(handle, SetupPacket, (void *)buf, 32, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(handle, &res);
    if (retVal < 0)
        return (int)retVal;
    
    if (res != TouCAN_RETVAL_OK)
        return (int)TOUCAN_ERROR_OFFSET - (int)res;
    
    if (str)
        strncpy(str, (const char *)buf, (size_t)size);
    
    return retVal;

}

int TouCAN_get_interface_transmit_delay(CANUSB_Handle_t handle, UInt8 channel,UInt32 *delay) {
    return (int)TOUCAN_ERROR_OFFSET;
}

int TouCAN_set_interface_transmit_delay(CANUSB_Handle_t handle, UInt8 channel,UInt32 *delay) {
    return (int)TOUCAN_ERROR_OFFSET;
}

///////////////////////////////////////// LIST, MASK ////////////////////////////////////////////
int TouCAN_set_filter_std_list_mask(CANUSB_Handle_t handle, Filter_Type_TypeDef type, UInt32 list, UInt32 mask) {
    return (int)TOUCAN_ERROR_OFFSET;
}

int TouCAN_set_filter_ext_list_mask(CANUSB_Handle_t handle, Filter_Type_TypeDef type, UInt32 list, UInt32 mask) {
    return (int)TOUCAN_ERROR_OFFSET;
}

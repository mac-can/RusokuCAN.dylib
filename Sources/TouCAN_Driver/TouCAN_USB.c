/*
 * TouCAN - macOS User-Space Driver for Rusoku TouCAN USB Adapters
 *
 * Copyright (C) 2020  Uwe Vogt, UV Software, Berlin (info@mac-can.com)
 *
 * The following source code was copied from CANAL-DLL for Windows
 * and modified to run under macOS 10.15 and later (x86_64 architecture)
 *
 * Copyright (C) 2018 Gediminas Simanskis (gediminas@rusoku.com)
 *
 * The original CANAL interface DLL for RUSOKU technologies for TouCAN,
 * TouCAN Marine, TouCAN Duo USB to CAN bus converter is licensed
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; version 3.0 of the License.
 *
 * CANAL-DLL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with CANAL-DLL.
 */
#include "TouCAN_USB.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/////////////////////////////////////////////////////
// USB request types & mask defines

#define        USB_HOST_TO_DEVICE                          0x00
#define        USB_DEVICE_TO_HOST                          0x80

#define        USB_REQ_TYPE_STANDARD                       0x00
#define        USB_REQ_TYPE_CLASS                          0x20
#define        USB_REQ_TYPE_VENDOR                         0x40
#define        USB_REQ_TYPE_MASK                           0x60

#define        USB_REQ_RECIPIENT_DEVICE                    0x00
#define        USB_REQ_RECIPIENT_INTERFACE                 0x01
#define        USB_REQ_RECIPIENT_ENDPOINT                  0x02
#define        USB_REQ_RECIPIENT_MASK                      0x03

/////////////////////////////////////////////////////
// TouCAN requests types (Command)

#define        TouCAN_RESET                                0x00 // OK
#define        TouCAN_CAN_INTERFACE_INIT                   0x01 // OK
#define        TouCAN_CAN_INTERFACE_DEINIT                 0x02 // OK
#define        TouCAN_CAN_INTERFACE_START                  0x03 // OK
#define        TouCAN_CAN_INTERFACE_STOP                   0x04 // OK

#define        TouCAN_FILTER_STD_ACCEPT_ALL                0x05
#define        TouCAN_FILTER_STD_REJECT_ALL                0x06

#define        TouCAN_FILTER_EXT_ACCEPT_ALL                0x07
#define        TouCAN_FILTER_EXT_REJECT_ALL                0x08

#define        TouCAN_SET_FILTER_STD_LIST_MASK             0x09
#define        TouCAN_SET_FILTER_EXT_LIST_MASK             0x0A
#define        TouCAN_GET_FILTER_STD_LIST_MASK             0x0B
#define        TouCAN_GET_FILTER_EXT_LIST_MASK             0x0C

#define        TouCAN_GET_CAN_ERROR_STATUS                 0x0D  // OK   - CAN_Error_Status(&hcan1)
#define        TouCAN_CLEAR_CAN_ERROR_STATUS               0x0D  // NOK  - CAN_Error_Status(&hcan1) // nenusistato
#define        TouCAN_GET_STATISTICS                       0x0E  // OK (VSCP CAN state)
#define        TouCAN_CLEAR_STATISTICS                     0x0F  // OK (VSCP CAN state)
#define        TouCAN_GET_HARDWARE_VERSION                 0x10  // OK
#define        TouCAN_GET_FIRMWARE_VERSION                 0x11  // OK
#define        TouCAN_GET_BOOTLOADER_VERSION               0x12  // OK
#define        TouCAN_GET_SERIAL_NUMBER                    0x13  // OK
//#define        TouCAN_SET_SERIAL_NUMBER                    0x14
//#define        TouCAN_RESET_SERIAL_NUMBER                  0x15
#define        TouCAN_GET_VID_PID                          0x16  // OK
#define        TouCAN_GET_DEVICE_ID                        0x17  // OK
#define        TouCAN_GET_VENDOR                           0x18  // OK

#define        TouCAN_GET_LAST_ERROR_CODE                  0x20  // HAL return error code    8bit // OK
#define        TouCAN_CLEAR_LAST_ERROR_CODE                0x21  // HAL return error code  8bit // OK
#define        TouCAN_GET_CAN_INTERFACE_STATE              0x22  // HAL_CAN_GetState(&hcan1)        8bit   // OK
#define        TouCAN_CLEAR_CAN_INTERFACE_STATE            0x23  // HAL_CAN_GetState(&hcan1); ----------------------------- NOK
#define        TouCAN_GET_CAN_INTERFACE_ERROR_CODE         0x24  // hcan->ErrorCode;    32bit    // OK  HAL_CAN_GetError(&hcan1);
#define        TouCAN_CLEAR_CAN_INTERFACE_ERROR_CODE       0x25  // hcan->ErrorCode;    32bit    // OK HAL_CAN_GetError(&hcan1);

#define        TouCAN_SET_CAN_INTERFACE_DELAY              0x26  // OK
#define        TouCAN_GET_CAN_INTERFACE_DELAY              0x27  // OK

/////////////////////////////////////////////////////
// TouCAN HAL return error codes

typedef enum
{
    HAL_OK =      0x00U,
    HAL_ERROR =   0x01U,
    HAL_BUSY =    0x02U,
    HAL_TIMEOUT = 0x03U
} HAL_StatusTypeDef;


/////////////////////////////////////////////////////
// TouCAN CAN interface state

typedef enum
{
    HAL_CAN_STATE_RESET =            0x00U,  /* CAN not yet initialized or disabled */
    HAL_CAN_STATE_READY =            0x01U,  /* CAN initialized and ready for use   */
    HAL_CAN_STATE_LISTENING =        0x02U,  /* CAN receive process is ongoing      */
    HAL_CAN_STATE_SLEEP_PENDING =    0x03U,  /* CAN sleep request is pending        */
    HAL_CAN_STATE_SLEEP_ACTIVE =     0x04U,  /* CAN sleep mode is active            */
    HAL_CAN_STATE_ERROR =            0x05U   /* CAN error state                     */

} HAL_CAN_StateTypeDef;


//////////////////////////////////////////////////////////////////////
// TouCAN init
//

int TouCAN_init(CANUSB_Handle_t hDevice, UInt16 u16Brp, UInt8 u8Tseg1, UInt8 u8Tseg2, UInt8 u8Sjw, UInt32 u32Flags) {
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
    data[0] = u8Tseg1;
    // tseg2
    data[1] = u8Tseg2;
    // sjw
    data[2] = u8Sjw;
    // Brp

    data[3] = (UInt8) ((u16Brp >> 8) & 0xFF);
    data[4] = (UInt8) (u16Brp & 0xFF);

    // flags
    data[5] = (UInt8) ((u32Flags >> 24) & 0xFF);
    data[6] = (UInt8) ((u32Flags >> 16) & 0xFF);
    data[7] = (UInt8) ((u32Flags >> 8) & 0xFF);
    data[8] = (UInt8)  (u32Flags & 0xFF);

    // TouCAN_CAN_INTERFACE_INIT
    retVal = CANUSB_DeviceRequest(hDevice, SetupPacket, (void *)data, 9, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(hDevice, &res);
    if (retVal < 0)
        return (int)retVal;
    
    if (res != TouCAN_RETVAL_OK)
        return (int)TOUCAN_ERROR_OFFSET - (int)res;
    
    return (int)TOUCAN_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// TouCAN deinit
//

int TouCAN_deinit(CANUSB_Handle_t hDevice) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 res;

    SetupPacket.RequestType = USB_HOST_TO_DEVICE | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_CAN_INTERFACE_DEINIT;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 0;
    
    // TouCAN_CAN_INTERFACE_DEINIT
    retVal = CANUSB_DeviceRequest(hDevice, SetupPacket, NULL, 0, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(hDevice, &res);
    if (retVal < 0)
        return (int)retVal;
    
    if (res != TouCAN_RETVAL_OK)
        return (int)TOUCAN_ERROR_OFFSET - (int)res;
    
    return (int)TOUCAN_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// TouCAN start
//

int TouCAN_start(CANUSB_Handle_t hDevice) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 res;

    SetupPacket.RequestType = USB_HOST_TO_DEVICE | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_CAN_INTERFACE_START;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 0;
    
    // TouCAN_CAN_INTERFACE_START
    retVal = CANUSB_DeviceRequest(hDevice, SetupPacket, NULL, 0, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(hDevice, &res);
    if (retVal < 0)
        return (int)retVal;
    
    if (res != TouCAN_RETVAL_OK)
        return (int)TOUCAN_ERROR_OFFSET - (int)res;
    
    return (int)TOUCAN_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// TouCAN stop
//

int TouCAN_stop(CANUSB_Handle_t hDevice) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 res;

    SetupPacket.RequestType = USB_HOST_TO_DEVICE | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_CAN_INTERFACE_STOP;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 0;

    // TouCAN_CAN_INTERFACE_STOP
    retVal = CANUSB_DeviceRequest(hDevice, SetupPacket, NULL, 0, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(hDevice, &res);
    if (retVal < 0)
        return (int)retVal;
    
    if (res != TouCAN_RETVAL_OK)
        return (int)TOUCAN_ERROR_OFFSET - (int)res;
    
    return (int)TOUCAN_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// TouCAN HAL get last error
//

int TouCAN_get_last_error_code(CANUSB_Handle_t hDevice, UInt8 *res) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt32 wLenDone;
    UInt8 LastErrorCode;

    SetupPacket.RequestType = USB_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_GET_LAST_ERROR_CODE;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 1;

    retVal = CANUSB_DeviceRequest(hDevice, SetupPacket, (void *)&LastErrorCode, 1, &wLenDone);
    if (retVal < 0)
        return (int)retVal;
    
    if ((wLenDone != 1) && (LastErrorCode != HAL_OK))
        return (int)TOUCAN_ERROR_OFFSET;
    
    if (res)
        *res = LastErrorCode;
    
    return (int)TOUCAN_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////
// TouCAN get CAN interface ERROR:  hcan->ErrorCode;
//

int TouCAN_get_interface_error_code(CANUSB_Handle_t hDevice, UInt32 *ErrorCode) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 buf[4];
    UInt8 res;

    SetupPacket.RequestType = USB_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_GET_CAN_INTERFACE_ERROR_CODE;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 4;

    retVal = CANUSB_DeviceRequest(hDevice, SetupPacket, (void *)buf, 4, NULL);
    if (retVal < 0)
        return (int)retVal;

    retVal = TouCAN_get_last_error_code(hDevice, &res);
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

int TouCAN_clear_interface_error_code(CANUSB_Handle_t hDevice) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 res;

    SetupPacket.RequestType = USB_HOST_TO_DEVICE | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_CLEAR_CAN_INTERFACE_ERROR_CODE;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 0;

    retVal = CANUSB_DeviceRequest(hDevice, SetupPacket, NULL, 0, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(hDevice, &res);
    if (retVal < 0)
        return (int)retVal;
    
    if (res != TouCAN_RETVAL_OK)
        return (int)TOUCAN_ERROR_OFFSET - (int)res;
    
    return (int)TOUCAN_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// TouCAN TouCAN_GET_CAN_INTERFACE_STATE   hcan->State;
//

int TouCAN_get_interface_state(CANUSB_Handle_t hDevice, UInt8 *state) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 buf[1];
    UInt8 res;

    SetupPacket.RequestType = USB_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_GET_CAN_INTERFACE_STATE;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 1;

    retVal = CANUSB_DeviceRequest(hDevice, SetupPacket, (void *)buf, 1, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(hDevice, &res);
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

int TouCAN_get_statistics(CANUSB_Handle_t hDevice, PCANALSTATISTICS statistics) {
    return (int)TOUCAN_ERROR_OFFSET;
}

//////////////////////////////////////////////////////////////////////
// TouCAN_clear_statistics VSCP
//

int TouCAN_clear_statistics(CANUSB_Handle_t hDevice) {
    return (int)TOUCAN_ERROR_OFFSET;
}

//////////////////////////////////////////////////////////////////////
// TouCAN_get_status VSCP
//

int TouCAN_get_canal_status(CANUSB_Handle_t hDevice, canalStatus *status) {
    return (int)TOUCAN_ERROR_OFFSET;
}
#endif

//////////////////////////////////////////////////////////////////////
// TouCAN_get_hardware_version
//
 
int TouCAN_get_hardware_version(CANUSB_Handle_t hDevice, UInt32 *ver) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 buf[4];
    UInt8 res;

    SetupPacket.RequestType = USB_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_GET_HARDWARE_VERSION;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 4;

    retVal = CANUSB_DeviceRequest(hDevice, SetupPacket, (void *)buf, 4, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(hDevice, &res);
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
int TouCAN_get_firmware_version(CANUSB_Handle_t hDevice, UInt32 *ver) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 buf[4];
    UInt8 res;

    SetupPacket.RequestType = USB_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_GET_FIRMWARE_VERSION;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 4;

    retVal = CANUSB_DeviceRequest(hDevice, SetupPacket, (void *)buf, 4, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(hDevice, &res);
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
int TouCAN_get_bootloader_version(CANUSB_Handle_t hDevice, UInt32 *ver) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 buf[4];
    UInt8 res;

    SetupPacket.RequestType = USB_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_GET_BOOTLOADER_VERSION;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 4;

    retVal = CANUSB_DeviceRequest(hDevice, SetupPacket, (void *)buf, 4, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(hDevice, &res);
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
int TouCAN_get_serial_number(CANUSB_Handle_t hDevice, UInt32 *ver) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 buf[4];
    UInt8 res;

    SetupPacket.RequestType = USB_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_GET_SERIAL_NUMBER;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 4;

    retVal = CANUSB_DeviceRequest(hDevice, SetupPacket, (void *)buf, 4, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(hDevice, &res);
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
int TouCAN_get_vid_pid(CANUSB_Handle_t hDevice, UInt32 *ver) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 buf[4];
    UInt8 res;

    SetupPacket.RequestType = USB_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_GET_VID_PID;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 4;

    retVal = CANUSB_DeviceRequest(hDevice, SetupPacket, (void *)buf, 4, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(hDevice, &res);
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
int TouCAN_get_device_id(CANUSB_Handle_t hDevice, UInt32 *ver) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 buf[4];
    UInt8 res;

    SetupPacket.RequestType = USB_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_GET_DEVICE_ID;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 4;

    retVal = CANUSB_DeviceRequest(hDevice, SetupPacket, (void *)buf, 4, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(hDevice, &res);
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
int TouCAN_get_vendor(CANUSB_Handle_t hDevice, unsigned int size, char *str) {
    CANUSB_SetupPacket_t SetupPacket;
    CANUSB_Return_t retVal;
    UInt8 buf[32];
    UInt8 res;

    SetupPacket.RequestType = USB_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE;
    SetupPacket.Request = TouCAN_GET_VENDOR;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0;
    SetupPacket.Length = 32;  // FIXME: 4(?)

    retVal = CANUSB_DeviceRequest(hDevice, SetupPacket, (void *)buf, 32, NULL);
    if (retVal < 0)
        return (int)retVal;
    
    retVal = TouCAN_get_last_error_code(hDevice, &res);
    if (retVal < 0)
        return (int)retVal;
    
    if (res != TouCAN_RETVAL_OK)
        return (int)TOUCAN_ERROR_OFFSET - (int)res;
    
    if (str)
        strncpy(str, (const char *)buf, (size_t)size);
    
    return retVal;

}

int TouCAN_get_interface_transmit_delay(CANUSB_Handle_t hDevice, UInt8 channel,UInt32 *delay) {
    return (int)TOUCAN_ERROR_OFFSET;
}

int TouCAN_set_interface_transmit_delay(CANUSB_Handle_t hDevice, UInt8 channel,UInt32 *delay) {
    return (int)TOUCAN_ERROR_OFFSET;
}

///////////////////////////////////////// LIST, MASK ////////////////////////////////////////////
int TouCAN_set_filter_std_list_mask(CANUSB_Handle_t hDevice, Filter_Type_TypeDef type, UInt32 list, UInt32 mask) {
    return (int)TOUCAN_ERROR_OFFSET;
}

int TouCAN_set_filter_ext_list_mask(CANUSB_Handle_t hDevice, Filter_Type_TypeDef type, UInt32 list, UInt32 mask) {
    return (int)TOUCAN_ERROR_OFFSET;
}

// ///////////////////////////////////////////////////////////////////
// MacCAN-TouCAN reception and transmission (asynchronous)
//
#define TOUCAN_MSG_STD_FRAME  (UInt8)0x00  // CANAL_IDFLAG_STANDARD
#define TOUCAN_MSG_XTD_FRAME  (UInt8)0x01  // CANAL_IDFLAG_EXTENDED
#define TOUCAN_MSG_RTR_FRAME  (UInt8)0x02  // CANAL_IDFLAG_RTR
#define TOUCAN_MSG_STS_FRAME  (UInt8)0x04  // CANAL_IDFLAG_STATUS

#define TOUCAN_STS_OK         (UInt8)0x00  // CANAL_STATUSMSG_OK
#define TOUCAN_STS_OVERRUN    (UInt8)0x01  // CANAL_STATUSMSG_OVERRUN
#define TOUCAN_STS_BUSLIGHT   (UInt8)0x02  // CANAL_STATUSMSG_BUSLIGHT
#define TOUCAN_STS_BUSHEAVY   (UInt8)0x03  // CANAL_STATUSMSG_BUSHEAVY
#define TOUCAN_STS_BUSOFF     (UInt8)0x04  // CANAL_STATUSMSG_BUSOFF
#define TOUCAN_STS_STUFF      (UInt8)0x20  // CANAL_STATUSMSG_STUFF
#define TOUCAN_STS_FORM       (UInt8)0x21  // CANAL_STATUSMSG_FORM
#define TOUCAN_STS_ACK        (UInt8)0x23  // CANAL_STATUSMSG_ACK
#define TOUCAN_STS_BIT1       (UInt8)0x24  // CANAL_STATUSMSG_BIT1
#define TOUCAN_STS_BIT0       (UInt8)0x25  // CANAL_STATUSMSG_BIT0
#define TOUCAN_STS_CRC        (UInt8)0x27  // CANAL_STATUSMSG_CRC

static void ReceptionCallback(void *refCon, UInt8 *buffer, UInt32 length) {
    CANUSB_UsbPipe_t *usbPipe = (CANUSB_UsbPipe_t *)refCon;
    MacCAN_Message_t message;
    UInt32 index = 0;
    
    assert(refCon);
    assert(buffer);
    
    while (length >= TOUCAN_USB_RX_DATA_FRAME_SIZE) {
        bzero(&message, sizeof(MacCAN_Message_t));
        (void) TouCAN_DecodeMessage(&message, &buffer[index], (CANUSB_MsgParam_t)usbPipe->ptrParam);
        (void) CANUSB_Enqueue(usbPipe, &message);
        index += TOUCAN_USB_RX_DATA_FRAME_SIZE;
        length -= TOUCAN_USB_RX_DATA_FRAME_SIZE;
    }
}

int TouCAN_StartReception(CANUSB_Handle_t hDevice, CANUSB_UsbPipe_t *usbPipe, const TouCAN_MsgParam_t *msgParam) {
    /* set up context for USB receive notifications */
    CANUSB_PIPE_CONTEXT(usbPipe, hDevice, TOUCAN_USB_RX_DATA_PIPE_REF, ReceptionCallback, (CANUSB_MsgParam_t)msgParam);
    /* start asynchronous read on endpoint #2 */
    return (int)CANUSB_ReadPipeAsyncStart(hDevice, TOUCAN_USB_RX_DATA_PIPE_REF, usbPipe);
}

int TouCAN_AbortReception(CANUSB_Handle_t hDevice) {
    /* stop asynchronous read on endpoint #2 */
    return CANUSB_ReadPipeAsyncAbort(hDevice, TOUCAN_USB_RX_DATA_PIPE_REF);;
}

int TouCAN_EncodeMessage(UInt8 *buffer, const MacCAN_Message_t *message) {
    int index = 0;
    
    assert(buffer);
    assert(message);

    /* byte 0: transmission flags */
    buffer[index++] = (UInt8)(message->xtd ? TOUCAN_MSG_XTD_FRAME : TOUCAN_MSG_STD_FRAME)
                    | (UInt8)(message->rtr ? TOUCAN_MSG_RTR_FRAME : TOUCAN_MSG_STD_FRAME);
    /* byte 1 - 4: identifier (big endian) */
    buffer[index++] = (UInt8)(message->id >> 24);
    buffer[index++] = (UInt8)(message->id >> 16);
    buffer[index++] = (UInt8)(message->id >> 8);
    buffer[index++] = (UInt8)message->id;
    /* byte 5: data length code (0..8) */
    buffer[index++] = (UInt8)message->dlc;
    /* byte 6 - 13: payload (8 bytes) */
    for (int i = 0; i < TOUCAN_USB_MAX_FRAME_LEN; i++)
        buffer[index++] = message->data[i];
    /* byte 14 - 17: timestamp? (big endian) */
    buffer[index++] = 0x00U;
    buffer[index++] = 0x00U;
    buffer[index++] = 0x00U;
    buffer[index++] = 0x00U;
    
    return index;
}

int TouCAN_DecodeMessage(MacCAN_Message_t *message, const UInt8 *buffer, TouCAN_MsgParam_t *param) {
    int index = 0;
    UInt64 hw_usec = 0U;
    UInt64 sw_usec = 0U;
    struct timespec ts;
    
    assert(buffer);
    assert(message);
    bzero(message, sizeof(MacCAN_Message_t));
    
    /* get system time (in [usec]) */
    (void) clock_gettime(CLOCK_MONOTONIC, &ts);
    sw_usec = ((UInt64)ts.tv_sec * 1000000U)
            + ((UInt64)ts.tv_nsec / 1000U);
    
    /* byte 0: transmission flags */
    message->xtd = (buffer[index] & TOUCAN_MSG_XTD_FRAME) ? 1 : 0;
    message->rtr = (buffer[index] & TOUCAN_MSG_RTR_FRAME) ? 1 : 0;
    message->sts = (buffer[index++] & TOUCAN_MSG_STS_FRAME) ? 1 : 0;
    /* byte 1 - 4: identifier (big endian) */
    message->id |= (UInt32)buffer[index++] << 24;
    message->id |= (UInt32)buffer[index++] << 16;
    message->id |= (UInt32)buffer[index++] << 8;
    message->id |= (UInt32)buffer[index++];
    /* byte 5: data length code (0..8) */
    message->dlc = (UInt8)buffer[index++];
    /* byte 6 - 13: payload (8 bytes) */
    for (int i = 0; i < TOUCAN_USB_MAX_FRAME_LEN; i++)
        message->data[i] = buffer[index++];
    /* byte 14 - 17: timestamp (big endian) */
    hw_usec |= (UInt64)buffer[index++] << 24;
    hw_usec |= (UInt64)buffer[index++] << 16;
    hw_usec |= (UInt64)buffer[index++] << 8;
    hw_usec |= (UInt64)buffer[index++];
    
    /* overflow handling (2^32 - 1 = 1:11:34.962296) */
    if (param) {
        if (param->m_u64StartTime == 0) {
            param->m_u64StartTime = sw_usec - hw_usec;
        }
        UInt64 diff_time = sw_usec - param->m_u64StartTime;
        hw_usec += ((diff_time / 0x100000000U) * 0x100000000U);
    }
    /* timestamp as struct timespec (fraction in [nsec]) */
    message->timestamp.tv_sec = (time_t)(hw_usec / 1000000U);
    message->timestamp.tv_nsec =  (long)(hw_usec % 1000000U) * (long)1000;
    if (message->timestamp.tv_nsec >= (long)1000000000) {
        message->timestamp.tv_nsec %= (long)1000000000;
        message->timestamp.tv_sec += (time_t)1;
    }
    /* status frame: data[0...] */
    if (message->sts && message->dlc && param) {
        switch (message->data[0]) {
        case TOUCAN_STS_OK:  // Normal condition.
            param->m_u8StatusByte = 0x00U;
            break;
        case TOUCAN_STS_OVERRUN:  // Overrun occured when sending data to CAN bus.
            //param->m_u8StatusByte |= ...;
            break;
        case TOUCAN_STS_BUSLIGHT:  // Error counter has reached 96.
            if (param->m_u8StatusByte & CANSTAT_EWRN)
                param->m_u8StatusByte &= ~CANSTAT_EWRN;
            break;
        case TOUCAN_STS_BUSHEAVY:  // Error counter has reached 128.
            param->m_u8StatusByte |= CANSTAT_EWRN;
            break;
        case TOUCAN_STS_BUSOFF:  // Device is in BUSOFF. CANAL_STATUS_OK is
                                 // sent when returning to operational mode.
            param->m_u8StatusByte |= CANSTAT_BOFF;
            break;
        case TOUCAN_STS_STUFF:  // Stuff Error.
        case TOUCAN_STS_FORM:  // Form Error.
        case TOUCAN_STS_ACK:  // Ack Error.
        case TOUCAN_STS_BIT1:  // Bit1 Error.
        case TOUCAN_STS_BIT0:  // Bit0 Error.
        case TOUCAN_STS_CRC:  // CRC Error.
            param->m_u8StatusByte |= CANSTAT_BERR;
            break;
        }
    }
    return index;
}

static int TouCAN_ResetDevice(CANUSB_Handle_t hDevice) {
    int retVal;
    UInt8 state = 0;
    UInt32 error = 0;
    
    /* get device state */
    retVal = TouCAN_get_interface_state(hDevice, &state);
    if (retVal < 0)
        return retVal;
    /* state LISTENING ==> state READY */
    if (state == (UInt8)HAL_CAN_STATE_LISTENING) {
        retVal = TouCAN_stop(hDevice);
        if (retVal < 0)
            return retVal;
        /* get new device state */
        retVal = TouCAN_get_interface_state(hDevice, &state);
        if (retVal < 0)
            return retVal;
    }
    /* state READY++ ==> state RESET */
    if (state != (UInt8)HAL_CAN_STATE_RESET) {
        retVal = TouCAN_deinit(hDevice);
        if (retVal < 0)
            return retVal;
    }
    /* get interface error code */
    retVal = TouCAN_get_interface_error_code(hDevice, &error);
    if (retVal < 0)
        return retVal;
    /* clear pending error(s) */
    if (error != (UInt32)HAL_CAN_ERROR_NONE) {
        retVal = TouCAN_clear_interface_error_code(hDevice);
    }
    return retVal;
}

int TouCAN_InitializeInterface(CANUSB_Handle_t hDevice) {
    int retVal;
    
    /* reset state and pending errors */
    retVal = TouCAN_ResetDevice(hDevice);
    if (retVal < 0)
        return retVal;
    /* initialize with default bit-rate and mode-flags */
    /* note: CAN API provides this at a later stage */
    retVal = TouCAN_init(hDevice, 10U, 14U, 5U, 4U, 0x00000000U);
    
    return retVal;
}

int TouCAN_TeardownInterface(CANUSB_Handle_t hDevice) {
    /* enter RESET state (deinit) */
    return TouCAN_ResetDevice(hDevice);
}

int TouCAN_SetBitrateAndMode(CANUSB_Handle_t hDevice, const MacCAN_Bitrate_t *bitrate,
                                                      const MacCAN_OpMode_t *opMode) {
    int retVal;
    UInt32 modeFlags = 0x00000000U;

    assert(opMode);
    assert(bitrate);

    modeFlags |= opMode->mon ? TouCAN_ENABLE_SILENT_MODE : 0x00000000U;
    modeFlags |= opMode->err ? TouCAN_ENABLE_STATUS_MESSAGES : 0x00000000U;
    // TODO: modeFlags |= opMode->nrtr
    // TODO: modeFlags |= opMode->nxtd

    /* reset state and pending errors */
    retVal = TouCAN_ResetDevice(hDevice);
    if (retVal < 0)
        return retVal;
    /* initialize with demanded bit-rate and mode-flags */
    retVal = TouCAN_init(hDevice,
                         bitrate->btr.nominal.brp,
                         bitrate->btr.nominal.tseg1,
                         bitrate->btr.nominal.tseg2,
                         bitrate->btr.nominal.sjw,
                         modeFlags);
    return retVal;
}

int TouCAN_StartInterface(CANUSB_Handle_t hDevice) {
    /* enter LISTENING state */
    return TouCAN_start(hDevice);
}

int TouCAN_StopInterface(CANUSB_Handle_t hDevice) {
    /* enter READY state again */
    return TouCAN_stop(hDevice);
}

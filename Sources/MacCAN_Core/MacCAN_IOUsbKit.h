/*
 *  MacCAN - macOS User-Space Driver for CAN to USB Interfaces
 *
 *  Copyright (C) 2012-2020  Uwe Vogt, UV Software, Berlin (info@mac-can.com)
 *
 *  This file is part of MacCAN-Core.
 *
 *  MacCAN-Core is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  MacCAN-Core is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with MacCAN-Core.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MACCAN_IOUSBKIT_H_INCLUDED
#define MACCAN_IOUSBKIT_H_INCLUDED

#include <MacTypes.h>

#ifndef CANUSB_MAX_DEVICES
#define CANUSB_MAX_DEVICES  42
#endif
#define CANUSB_INVALID_HANDLE  (-1)

typedef int CANUSB_Handle_t;
typedef int CANUSB_Return_t;

typedef void (*CANUSB_Callback_t)(void *refCon, UInt8 *buffer, UInt32 length);

typedef void *CANUSB_MsgParam_t;

typedef struct msg_queue_t_ {
    UInt32 size;
    UInt32 used;
    UInt32 head;
    UInt32 tail;
    void *queueElem;
    size_t elemSize;
    struct cond_wait_t {
        pthread_mutex_t mutex;
        pthread_cond_t cond;
        Boolean flag;
    } wait;
    struct overflow_t {
        Boolean flag;
        UInt64 counter;
    } ovfl;
} CANUSB_MsgQueue_t;

typedef struct usb_buffer_t_ {
    UInt8 *data[2];
    UInt8 index;
    UInt32 size;
} CANUSB_Buffer_t;

typedef struct usb_pipe_t_ {
    UInt8 pipeRef;
    CANUSB_Handle_t handle;
    CANUSB_Buffer_t buffer;
    CANUSB_Callback_t callback;
    CANUSB_MsgQueue_t msgQueue;
    CANUSB_MsgParam_t ptrParam;
    Boolean running;
} CANUSB_UsbPipe_t;

typedef struct usb_setup_packet_t_ {
    UInt8  RequestType;
    UInt8  Request;
    UInt16 Value;
    UInt16 Index;
    UInt16 Length;
} CANUSB_SetupPacket_t;

#ifdef __cplusplus
extern "C" {
#endif

extern CANUSB_Return_t CANUSB_Initialize(void);

extern CANUSB_Return_t CANUSB_Teardown(void);

extern CANUSB_Handle_t CANUSB_OpenDevice(UInt16 vendorId, UInt16 productId, UInt8 channelNo);

extern CANUSB_Return_t CANUSB_CloseDevice(CANUSB_Handle_t handle);

extern CANUSB_Return_t CANUSB_DeviceRequest(CANUSB_Handle_t handle, CANUSB_SetupPacket_t setupPacket, void *buffer, UInt16 size, UInt32 *transferred);

extern CANUSB_Return_t CANUSB_ReadPipe(CANUSB_Handle_t handle, UInt8 pipeRef, void *buffer, UInt32 *size);

extern CANUSB_Return_t CANUSB_ReadPipeAsyncStart(CANUSB_Handle_t handle, UInt8 pipeRef, CANUSB_UsbPipe_t *usbPipe);

extern CANUSB_Return_t CANUSB_ReadPipeAsyncAbort(CANUSB_Handle_t handle, UInt8 pipeRef);

extern CANUSB_Return_t CANUSB_WritePipe(CANUSB_Handle_t handle, UInt8 pipeRef, void *buffer, UInt32 size);

extern CANUSB_Handle_t CANUSB_GetFirstDevice(void);

extern CANUSB_Handle_t CANUSB_GetNextDevice(void);

extern Boolean CANUSB_IsDevicePresent(CANUSB_Handle_t handle);

extern Boolean CANUSB_IsDeviceOpened(CANUSB_Handle_t handle);

extern CANUSB_Return_t CANUSB_GetDeviceName(CANUSB_Handle_t handle, char *buffer, size_t nbytes);

extern CANUSB_Return_t CANUSB_GetDeviceVendorId(CANUSB_Handle_t handle, UInt16 *value);

extern CANUSB_Return_t CANUSB_GetDeviceProductId(CANUSB_Handle_t handle, UInt16 *value);

extern CANUSB_Return_t CANUSB_GetDeviceReleaseNo(CANUSB_Handle_t handle, UInt16 *value);

extern CANUSB_Return_t CANUSB_GetDeviceLocation(CANUSB_Handle_t handle, UInt32 *value);

extern CANUSB_Return_t CANUSB_GetDeviceAddress(CANUSB_Handle_t handle, UInt16 *value);

extern CANUSB_Return_t CANUSB_GetInterfaceClass(CANUSB_Handle_t handle, UInt8 *value);

extern CANUSB_Return_t CANUSB_GetInterfaceSubClass(CANUSB_Handle_t handle, UInt8 *value);

extern CANUSB_Return_t CANUSB_GetInterfaceProtocol(CANUSB_Handle_t handle, UInt8 *value);

extern CANUSB_Return_t CANUSB_GetInterfaceNumEndpoints(CANUSB_Handle_t handle, UInt8 *value);

extern UInt32 CANUSB_GetVersion(void);

#ifdef __cplusplus
}
#endif
#endif /* MACCAN_IOUSBKIT_H_INCLUDED */

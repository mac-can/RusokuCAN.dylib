/*  SPDX-License-Identifier: GPL-3.0-or-later */
/*
 *  TouCAN - macOS User-Space Driver for Rusoku TouCAN USB Adapters
 *
 *  Copyright (C) 2021  Uwe Vogt, UV Software, Berlin (info@mac-can.com)
 *
 *  This file is part of MacCAN-TouCAN.
 *
 *  MacCAN-TouCAN is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  MacCAN-TouCAN is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with MacCAN-TouCAN.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "TouCAN_USB_Device.h"
#include "MacCAN_Debug.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

static CANUSB_Return_t GetUsbConfiguration(CANUSB_Handle_t handle, TouCAN_Device_t *device) {
    CANUSB_Return_t retVal = CANUSB_ERROR_FATAL;
//    uint8_t dir, type;
//    uint16_t size;

    /* sanity check */
    if (!device)
        return CANUSB_ERROR_NULLPTR;
    if (device->configured)
        return CANUSB_ERROR_YETINIT;

    /* get product ID and revision no. from device */
    device->handle = CANUSB_INVALID_HANDLE;
    if ((retVal = CANUSB_GetDeviceProductId(handle, &device->productId)) < 0)
        return retVal;
    if ((retVal = CANUSB_GetDeviceReleaseNo(handle, &device->releaseNo)) < 0)
        return retVal;

#if (0)
    /* get number of CAN channels from device */
    if ((retVal = CANUSB_GetDeviceNumCanChannels(handle, &device->numChannels)) < 0)
        return retVal;
    /* set CAN channel number to be used */
    if ((channel < device->numChannels) && (channel < KVASER_MAX_CAN_CHANNELS))
        device->channelNo = channel;
    else
        return CANUSB_ERROR_FATAL;

    /* get number of endpoints from device */
    if ((retVal = CANUSB_GetInterfaceNumEndpoints(handle, &device->endpoints.numEndpoints)) < 0)
        return retVal;
    /* get endpoint properties from device */
    // FIXME: Nah, nah, there's gotta be something better!
    for (uint8_t i = 1U; (i <= device->endpoints.numEndpoints) && (i < (1U + (TOUCAN_MAX_CAN_CHANNELS * 2U))); i++) {
        if (CANUSB_GetInterfaceEndpointDirection(handle, i, &dir) < 0)
            return retVal;
        if (CANUSB_GetInterfaceEndpointTransferType(handle, i, &type) < 0)
            return retVal;
        if (CANUSB_GetInterfaceEndpointMaxPacketSize(handle, i, &size) < 0)
            return retVal;
        if ((i % 2)) {  /* bulk in: odd endpoint numbers {1..} */
            if ((dir == USBPIPE_DIR_IN) && (type == USBPIPE_TYPE_BULK))
                device->endpoints.bulkIn.packetSize = size;
            else
                device->endpoints.bulkIn.packetSize = 0U;
            device->endpoints.bulkIn.pipeRef = i;
        } else {  /* bulk out: even endpoint numbers {2..} */
            if ((dir == USBPIPE_DIR_OUT) && (type == USBPIPE_TYPE_BULK))
                device->endpoints.bulkOut.packetSize = size;
            else
                device->endpoints.bulkOut.packetSize = 0U;
            device->endpoints.bulkOut.pipeRef = i;
        }
    }
#endif
    /* set device name, vendor name and website (zero-terminated strings) */
    if (CANUSB_GetDeviceName(handle, device->name, TOUCAN_MAX_NAME_LENGTH) < 0)
        strncpy(device->name, "(unkown)", TOUCAN_MAX_NAME_LENGTH);
    strncpy(device->vendor, TOUCAN_VENDOR_NAME, TOUCAN_MAX_NAME_LENGTH);
    strncpy(device->website, TOUCAN_VENDOR_URL, TOUCAN_MAX_NAME_LENGTH);

    /* the USB handle and the CAN channel number are valid now,
     * but the configuration must be confirmed for the device! */
    device->handle = handle;
    retVal = CANUSB_SUCCESS;

    return retVal;
}

CANUSB_Return_t TouCAN_ProbeUsbDevice(CANUSB_Index_t channel, uint16_t *productId) {
    CANUSB_Return_t retVal = CANUSB_ERROR_FATAL;
    CANUSB_Index_t index = channel;

    /* check if the device is present (available) and opened (occupied) */
    if (!CANUSB_IsDevicePresent(index)) {
//        MACCAN_DEBUG_INFO("+++ MacCAN-Core: device (%02x) not available\n", channel);
        retVal = CANERR_HANDLE;
    } else if (!CANUSB_IsDeviceOpened(index)) {
//        MACCAN_DEBUG_INFO("+++ MacCAN-Core: device (%02x) available\n", channel);
        retVal = CANERR_NOERROR;
    } else {
//        MACCAN_DEBUG_INFO("+++ MacCAN-Core: device (%02x) occupied\n", channel);
        retVal = CANERR_NOERROR + 1;
    }
    /* get the product ID of the USB device (NULL pointer is checked there) */
    (void)CANUSB_GetDeviceProductId(index, productId);

    return retVal;
}

CANUSB_Return_t TouCAN_OpenUsbDevice(CANUSB_Index_t channel, TouCAN_Device_t *device) {
    CANUSB_Return_t retVal = CANUSB_ERROR_FATAL;
    CANUSB_Handle_t handle = CANUSB_INVALID_HANDLE;
    CANUSB_Index_t index = channel;

    /* sanity check */
    if (!device)
        return CANUSB_ERROR_NULLPTR;
    if (device->configured)
        return CANUSB_ERROR_YETINIT;

    /* open the USB device at index, if and only if the vendor ID matches */
    handle = CANUSB_OpenDevice(index, TOUCAN_VENDOR_ID, CANUSB_ANY_PRODUCT_ID);
    if (handle < 0) {
//        MACCAN_DEBUG_ERROR("+++ MacCAN-Core: device could not be opened (%02x)\n", channel);
        return CANUSB_ERROR_NOTINIT;
    }
    /* get the USB configuration of the device */
    retVal = GetUsbConfiguration(handle, device);
    if (retVal < 0) {
//        MACCAN_DEBUG_ERROR("+++ MacCAN-Core: configuration could not be read (%02x)\n", channel);
        (void)CANUSB_CloseDevice(handle);
        return retVal;
    }
    /* create a message queue for received CAN frames */
    device->recvData.msgQueue = CANQUE_Create(TOUCAN_RCV_QUEUE_SIZE, sizeof(TouCAN_CanMessage_t));
    if (device->recvData.msgQueue == NULL) {
//        MACCAN_DEBUG_ERROR("+++ %s CAN%u: message queue could not be created (NULL)\n", device->name, device->channelNo+1);
        (void)CANUSB_CloseDevice(handle);
        return retVal;;
    }
    /* create a pipe context for the selected CAN channel on the device */
#if (0)
    uint8_t pipeRef = device->endpoints.bulkIn.pipeRef;
    size_t bufSize = device->endpoints.bulkIn.packetSize;
    device->recvPipe = CANUSB_CreatePipeAsync(device->handle, pipeRef, bufSize);
    // TODO: realize general MacCAN endpoint module
#else
    device->recvPipe = CANUSB_CreatePipeAsync(device->handle, TOUCAN_USB_RX_DATA_PIPE_REF, TOUCAN_USB_RX_DATA_PIPE_SIZE);
#endif
    if (device->recvPipe == NULL) {
//        MACCAN_DEBUG_ERROR("+++ %s CAN%u: asynchronous pipe context could not be created (NULL)\n", device->name, device->channelNo+1);
        (void)CANQUE_Destroy(device->recvData.msgQueue);
        (void)CANUSB_CloseDevice(handle);
        return CANUSB_ERROR_RESOURCE;
    }
    return retVal;
}

CANUSB_Return_t TouCAN_CloseUsbDevice(TouCAN_Device_t *device) {
    CANUSB_Return_t retVal = CANUSB_ERROR_FATAL;

    /* sanity check */
    if (!device)
        return CANUSB_ERROR_NULLPTR;
//    if (!device->configured)
//        return CANUSB_ERROR_NOTINIT;

    /* abort asynchronous pipe */
    /*retVal =*/ CANUSB_ReadPipeAsyncAbort(device->recvPipe);
//    if (retVal < 0)
//        MACCAN_DEBUG_ERROR("+++ %s CAN%u: asynchronous pipe could not be stopped (%i)\n", device->name, device->channelNo+1, retVal);
    /* close the USB device */
    retVal = CANUSB_CloseDevice(device->handle);
//    if (retVal < 0)
//        MACCAN_DEBUG_ERROR("+++ %s CAN%u: device could not be closed (%i)\n", device->name, device->channelNo+1, retVal);
    /* destroy the pipe context */
    /*retVal =*/ CANUSB_DestroyPipeAsync(device->recvPipe);
//    if (retVal < 0)
//        MACCAN_DEBUG_ERROR("+++ %s CAN%u: asynchronous pipe context could not be released (%i)\n", device->name, device->channelNo+1, retVal);
    /* destroy the message queue */
    /*retVal =*/ CANQUE_Destroy(device->recvData.msgQueue);
//    if (retVal < 0)
//        MACCAN_DEBUG_ERROR("+++ %s CAN%u: message queue could not be released (%i)\n", device->name, device->channelNo+1, retVal);
    /* Live long and prosper! */
    device->handle = CANUSB_INVALID_HANDLE;
    device->recvData.msgQueue = NULL;
    device->recvPipe = NULL;
    device->configured = false;

    return retVal;
}

CANUSB_Return_t TouCAN_StartReception(TouCAN_Device_t *device, CANUSB_Callback_t callback) {
    CANUSB_Return_t retVal = CANUSB_ERROR_FATAL;

    /* sanity check */
    if (!device)
        return CANUSB_ERROR_NULLPTR;
    if (!device->configured)
        return CANUSB_ERROR_NOTINIT;

    /* start asynchronous read on endpoint */
    retVal = CANUSB_ReadPipeAsyncStart(device->recvPipe, callback, (void*)&device->recvData);
//    if (retVal < 0)
//        MACCAN_DEBUG_ERROR("+++ %s #%u: reception loop could not be started (%i)\n", device->name, device->channelNo, retVal);

    return retVal;
}

CANUSB_Return_t TouCAN_AbortReception(TouCAN_Device_t *device) {
    CANUSB_Return_t retVal = CANUSB_ERROR_FATAL;

    /* sanity check */
    if (!device)
        return CANUSB_ERROR_NULLPTR;
    if (!device->configured)
        return CANUSB_ERROR_NOTINIT;

    /* stop asynchronous read on endpoint */
    retVal = CANUSB_ReadPipeAsyncAbort(device->recvPipe);
//    if (retVal < 0)
//        MACCAN_DEBUG_ERROR("+++ %s #%u: reception loop could not be stopped (%i)\n", device->name, device->channelNo, retVal);

    return retVal;
}


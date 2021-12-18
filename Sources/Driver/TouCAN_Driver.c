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
#include "TouCAN_Driver.h"

#include "MacCAN_Devices.h"
#include "TouCAN_USB_Driver.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

const CANDEV_Device_t CANDEV_Devices[] = {
    {TOUCAN_USB_VENDOR_ID, TOUCAN_USB_PRODUCT_ID, 1U},
    CANDEV_LAST_ENTRY_IN_DEVICE_LIST
};

CANUSB_Return_t TouCAN_ProbeChannel(TouCAN_Channel_t channel, TouCAN_OpMode_t mode, int *state) {
    CANUSB_Return_t retVal = CANUSB_ERROR_FATAL;
    TouCAN_OpMode_t capa = CANMODE_DEFAULT;
    uint16_t productId = 0xFFFFU;

    /* test USB device at given index (channel) if it is present and possibly opened */
    retVal = TouCAN_ProbeUsbDevice(channel, &productId);
    if (retVal < 0) {
        if (state)
            *state = CANBRD_NOT_PRESENT;
        return CANERR_NOERROR;
    } else {
        if (state)
            *state = retVal > 0 ? CANBRD_OCCUPIED : CANBRD_PRESENT;
        retVal = CANERR_NOERROR;
    }
    /* get operation capability of the appropriate CAN controller */
    switch (productId) {
        case TOUCAN_USB_PRODUCT_ID:
            TouCAN_USB_GetOperationCapability(&capa);
            break;
        default:
            /* unsupported TouCAN device */
            if (state)
                *state = CANBRD_NOT_TESTABLE;
            retVal = CANUSB_ERROR_FATAL;
            break;
    }
    /* check given operation mode against the operation capability */
    if ((retVal == CANERR_NOERROR) && ((mode & ~capa) != 0)) {
        retVal = CANERR_ILLPARA;
    }
    return retVal;
}

CANUSB_Return_t TouCAN_InitializeChannel(TouCAN_Channel_t channel, TouCAN_OpMode_t mode, TouCAN_Device_t *device) {
    CANUSB_Return_t retVal = CANUSB_ERROR_FATAL;

    /* open USB device at given index (channel) and allocate required resources (pipe context) */
    /* note: the device context is preinitialized, but must be confirmed by the CAN channel */
    retVal = TouCAN_OpenUsbDevice(channel, device);
    if (retVal < 0) {
        return retVal;;
    }
    switch (device->productId) {
        case TOUCAN_USB_PRODUCT_ID:
            /* configure and confirm device descriptor for TouCAN USB device and
             * initialize the CAN channel (CAN controller is in INIT state) */
            if (TouCAN_USB_ConfigureChannel(device))
                retVal = TouCAN_USB_InitializeChannel(device, mode);
            else
                retVal = CANUSB_ERROR_NOTINIT;
            break;
        default:
            /* oops, an unsupported Rusoku USB device was found!
             * Send me some Dollars and I will give my best. */
            retVal = CANUSB_ERROR_NOTINIT;
    }
    if (retVal < 0) {
        (void)TouCAN_CloseUsbDevice(device);
    }
    return retVal;
}

CANUSB_Return_t TouCAN_TeardownChannel(TouCAN_Device_t *device) {
    CANUSB_Return_t retVal = CANUSB_ERROR_FATAL;

    /* sanity check */
    if (!device)
        return CANUSB_ERROR_NULLPTR;
    if (!device->configured)
        return CANUSB_ERROR_NOTINIT;

    /* teardown the whole ... */
    switch (device->productId) {
        case TOUCAN_USB_PRODUCT_ID:
            retVal = TouCAN_USB_TeardownChannel(device);
            break;
    }
    /* close USB device and release all resources */
    /* note: don't care about the result */
    (void)TouCAN_CloseUsbDevice(device);

    return retVal;

}

CANUSB_Return_t TouCAN_SignalChannel(TouCAN_Device_t *device) {
    CANUSB_Return_t retVal = CANUSB_ERROR_FATAL;

    /* sanity check */
    if (!device)
        return CANUSB_ERROR_NULLPTR;
    if (!device->configured)
        return CANUSB_ERROR_NOTINIT;

    /* signal wait condition */
    switch (device->productId) {
        case TOUCAN_USB_PRODUCT_ID:
            retVal = CANQUE_Signal(device->recvData.msgQueue);
            break;
    }
    return retVal;
}

CANUSB_Return_t TouCAN_SetBitrate(TouCAN_Device_t *device, const TouCAN_Bitrate_t *bitrate) {
    CANUSB_Return_t retVal = CANUSB_ERROR_FATAL;

    /* sanity check */
    if (!device)
        return CANUSB_ERROR_NULLPTR;
    if (!device->configured)
        return CANUSB_ERROR_NOTINIT;

    /* set bit-rate settings */
    switch (device->productId) {
        case TOUCAN_USB_PRODUCT_ID:
            retVal = TouCAN_USB_SetBitrate(device, bitrate);
            break;
    }
    return retVal;
}

CANUSB_Return_t TouCAN_StartCan(TouCAN_Device_t *device) {
    CANUSB_Return_t retVal = CANUSB_ERROR_FATAL;

    /* sanity check */
    if (!device)
        return CANUSB_ERROR_NULLPTR;
    if (!device->configured)
        return CANUSB_ERROR_NOTINIT;

    /* start CAN controller (with configured settings) */
    switch (device->productId) {
        case TOUCAN_USB_PRODUCT_ID:
            retVal = TouCAN_USB_StartCan(device);
            break;
    }
    return retVal;
}

CANUSB_Return_t TouCAN_StopCan(TouCAN_Device_t *device) {
    CANUSB_Return_t retVal = CANUSB_ERROR_FATAL;

    /* sanity check */
    if (!device)
        return CANUSB_ERROR_NULLPTR;
    if (!device->configured)
        return CANUSB_ERROR_NOTINIT;

    /* reset CAN controller */
    switch (device->productId) {
        case TOUCAN_USB_PRODUCT_ID:
            retVal = TouCAN_USB_StopCan(device);
            break;
    }
    return retVal;
}

CANUSB_Return_t TouCAN_WriteMessage(TouCAN_Device_t *device, const TouCAN_CanMessage_t *message, uint16_t timeout) {
    CANUSB_Return_t retVal = CANUSB_ERROR_FATAL;

    /* sanity check */
    if (!device)
        return CANUSB_ERROR_NULLPTR;
    if (!device->configured)
        return CANUSB_ERROR_NOTINIT;

    /* send a CAN message */
    switch (device->productId) {
        case TOUCAN_USB_PRODUCT_ID:
            retVal = TouCAN_USB_WriteMessage(device, message, timeout);
            break;
    }
    return retVal;
}

CANUSB_Return_t TouCAN_ReadMessage(TouCAN_Device_t *device, TouCAN_CanMessage_t *message, uint16_t timeout) {
    CANUSB_Return_t retVal = CANUSB_ERROR_FATAL;

    /* sanity check */
    if (!device)
        return CANUSB_ERROR_NULLPTR;
    if (!device->configured)
        return CANUSB_ERROR_NOTINIT;

    /* read a CAN message from message queue, if any */
    switch (device->productId) {
        case TOUCAN_USB_PRODUCT_ID:
            retVal = TouCAN_USB_ReadMessage(device, message, timeout);
            break;
    }
    return retVal;
}

CANUSB_Return_t TouCAN_GetBusStatus(TouCAN_Device_t *device, TouCAN_Status_t *status) {
    CANUSB_Return_t retVal = CANUSB_ERROR_FATAL;

    /* sanity check */
    if (!device)
        return CANUSB_ERROR_NULLPTR;
    if (!device->configured)
        return CANUSB_ERROR_NOTINIT;

    /* get CAN bus status */
    switch (device->productId) {
        case TOUCAN_USB_PRODUCT_ID:
            retVal = TouCAN_USB_GetBusStatus(device, status);
            break;
    }
    return retVal;
}

bool TouCAN_Index2Bitrate(TouCAN_Device_t *device, int32_t index, TouCAN_Bitrate_t *bitrate) {
    bool retVal = false;

    /* sanity check */
    if (!device)
        return CANUSB_ERROR_NULLPTR;
    if (!device->configured)
        return CANUSB_ERROR_NOTINIT;

    /* map CiA CANopen index to bit-rate settings */
    switch (device->productId) {
        case TOUCAN_USB_PRODUCT_ID:
            retVal = TouCAN_USB_Index2Bitrate(index, bitrate);
            break;
    }
    return retVal;
}

/* ---  MacCAN IOUsbKit initialization  ---
 */
CANUSB_Return_t TouCAN_InitializeDriver(void) {
    /* initialize the MacCAN IOUsbKit */
    return CANUSB_Initialize();
}

CANUSB_Return_t TouCAN_TeardownDriver(void) {
    /* release the MacCAN IOUsbKit */
    return CANUSB_Teardown();
}

uint32_t TouCAN_DriverVersion(void) {
    /* version of the MacCAN IOUsbKit */
    return CANUSB_GetVersion();
}

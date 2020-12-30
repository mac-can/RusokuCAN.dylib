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
#include "MacCAN_IOUsbKit.h"
#include "MacCAN_Devices.h"
#include "MacCAN_Debug.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

#include <mach/mach.h>
#include <mach/clock.h>

#include <CoreFoundation/CFRunLoop.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFPlugInCOM.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOMessage.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/IOReturn.h>
#include <IOKit/IOTypes.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/usb/USBSpec.h>
#include <IOKit/usb/USB.h>

#define VERSION_MAJOR     0
#define VERSION_MINOR     1
#define VERSION_PATCH     0

/*#define PRINT_USB_PIPE_INFO*/

/* CAN API V3 compatible error codes */
#define CANUSB_ERROR_RESOURCE (-90)
#define CANUSB_ERROR_reserved (-91)
#define CANUSB_ERROR_HANDLE   (-92)
#define CANUSB_ERROR_ILLPARA  (-93)
#define CANUSB_ERROR_NULLPTR  (-94)
#define CANUSB_ERROR_NOTINIT  (-95)
#define CANUSB_ERROR_YETINIT  (-96)
#define CANUSB_ERROR_LIBRARY  (-97)
#define CANUSB_ERROR_NOTSUPP  (-98)
#define CANUSB_ERROR_FATAL    (-99)

#define IS_HANDLE_VALID(hnd)  ((0 <= (hnd)) && ((hnd) < CANUSB_MAX_DEVICES))

#define ENTER_CRITICAL_SECTION(idx)  assert(0 == pthread_mutex_lock(&usbDevice[idx].ptMutex))
#define LEAVE_CRITICAL_SECTION(idx)  assert(0 == pthread_mutex_unlock(&usbDevice[idx].ptMutex))

static void ReadPipeCallback(void *refCon, IOReturn result, void *arg0);
static int SetupDirectory(SInt32 vendorID, SInt32 productID);
static void DeviceAdded(void *refCon, io_iterator_t iterator);
static void DeviceRemoved(void *refCon, io_iterator_t iterator);
static IOReturn ConfigureDevice(IOUSBDeviceInterface **dev);
static IOReturn FindInterface(IOUSBDeviceInterface **device, int index);
static void* WorkerThread(void* arg);

typedef struct usb_interface_t_ {           /* USB interface: */
    Boolean fOpened;                        /*   interface is opened */
    UInt8 u8Class;                          /*   class of the interface (8-bit) */
    UInt8 u8SubClass;                       /*   subclass of the interface (8-bit) */
    UInt8 u8Protocol;                       /*   protocol of the interface (8-bit) */
    UInt8 u8NumEndpoints;                   /*   number of endpoints of the interface */
    IOUSBInterfaceInterface **ioInterface;  /*   interface interface (instance) */
    CANUSB_Handle_t hDevice;                /*   USB device handle (index) */
} USBInterface_t;

typedef struct usb_device_t_ {              /* USB device: */
    Boolean fPresent;                       /*   device is present */
    io_name_t szName;                       /*   device name */
    UInt16 u16VendorId;                     /*   vendor ID (16-bit) */
    UInt16 u16ProductId;                    /*   product ID (16-bit) */
    UInt16 u16ReleaseNo;                    /*   release no. (16-bit) */
    UInt32 u32Location;                     /*   unique location ID (32-bit) */
    UInt16 u16Address;                      /*   device address (16-bit?) */
    IOUSBDeviceInterface **ioDevice;        /*   device interface (instance) */
    USBInterface_t usbInterface/*[x]*/;     /*   interface interface (only first one supported) */
    pthread_mutex_t ptMutex;                /*   pthread mutex for mutual exclusion */
} USBDevice_t;

typedef struct usb_driver_t_ {              /* USB driver: */
    Boolean fRunning;                       /*   flag: driver running */
    pthread_t ptThread;                     /*   pthread of the driver */
    pthread_mutex_t ptMutex;                /*   pthread mutex for mutual exclusion */
    CFRunLoopRef refRunLoop;                /*   run loop of the driver */
    IONotificationPortRef refNotifyPort;    /*   port for notifications */
    io_iterator_t iterBulkDevicePlugged;    /*   iterator for plugged device(s) */
    io_iterator_t iterBulkDeviceUnplugged;  /*   iterator for unplugged device(s) */
} USBDriver_t;

static USBDriver_t usbDriver;
static USBDevice_t usbDevice[CANUSB_MAX_DEVICES];
static CANUSB_Handle_t idxDevice = 0;
static Boolean fInitialized = false;

CANUSB_Return_t CANUSB_Initialize(void) {
    int index, rc = -1;
    pthread_attr_t attr;
    time_t now;

    /* must not be initialized */
    if (fInitialized)
        return CANUSB_ERROR_YETINIT;
    
    /* initialize the driver and its devices */
    bzero(&usbDriver, sizeof(USBDriver_t));
    usbDriver.fRunning = false;
    for (index = 0; index < CANUSB_MAX_DEVICES; index++) {
        bzero(&usbDevice[index], sizeof(USBDevice_t));
        usbDevice[index].fPresent = false;
        /* create a mutex for each device */
        if((rc = pthread_mutex_init(&usbDevice[index].ptMutex, NULL)) < 0)
            goto error_initialize;
    }
    /* create a mutex and a thread for the driver */
    if ((rc = pthread_mutex_init(&usbDriver.ptMutex, NULL)) < 0)
        goto error_initialize;
    if ((rc = pthread_attr_init(&attr)) != 0)
        goto error_initialize;
    if ((rc = pthread_attr_setstacksize(&attr, 64*1024)) != 0)
        goto error_initialize;
    if ((rc = pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED)) != 0)
        goto error_initialize;
    if ((rc = pthread_attr_setschedpolicy(&attr, SCHED_RR)) != 0)
        goto error_initialize;
    rc = pthread_create(&usbDriver.ptThread, &attr, WorkerThread, NULL);
    assert(pthread_attr_destroy(&attr) == 0);
    if (rc != 0)
        goto error_initialize;

    /* wait for the driver being loaded (by the created thread) or timed out*/
    MACCAN_DEBUG_CORE("    Loading the MacCAN driver...\n");
    now = time(NULL);
    do {
        usleep(1000);
        assert(0 == pthread_mutex_lock(&usbDriver.ptMutex));
        fInitialized = usbDriver.fRunning;
        assert(0 == pthread_mutex_unlock(&usbDriver.ptMutex));
    } while (!fInitialized && (time(NULL) < now+5));
    return (fInitialized ? 0 : CANUSB_ERROR_NOTINIT);

error_initialize:
    /* on error: tidy-up! */
    for (index = index - 1; index >= 0; index--)
        (void) pthread_mutex_destroy(&usbDevice[index].ptMutex);
    /* the driver has not been loaded! */
    fInitialized = false;
    //return (-10000) + rc;
    return CANUSB_ERROR_NOTINIT;
}

CANUSB_Return_t CANUSB_Teardown(void) {
    int index;
    
    /* must be initialized */
    if (!fInitialized)
        return CANUSB_ERROR_NOTINIT;
    
    /* "Mr. Gorbachev, tear down this wall!" */
    MACCAN_DEBUG_CORE("    Release the MacCAN driver...\n");
    assert(pthread_cancel(usbDriver.ptThread) == 0);
    usleep(54945);
    
    /* close all USB devices */
    for (index = 0; index < CANUSB_MAX_DEVICES; index++) {
        /* release the USB device */
        //MACCAN_DEBUG_FUNC("lock #%i\n", index);
        ENTER_CRITICAL_SECTION(index);
        if (usbDevice[index].fPresent &&
            (usbDevice[index].ioDevice != NULL)) {
            MACCAN_DEBUG_CORE("    - Device #%i: %s", index, usbDevice[index].szName);
            if (usbDevice[index].usbInterface.fOpened) {
                /* close the USB interface interface(s) */
                if(usbDevice[index].usbInterface.ioInterface) {
                    MACCAN_DEBUG_CODE(0, "close and release I/O interface\n");
                    (void) (*usbDevice[index].usbInterface.ioInterface)->USBInterfaceClose(usbDevice[index].usbInterface.ioInterface);
                    (void) (*usbDevice[index].usbInterface.ioInterface)->Release(usbDevice[index].usbInterface.ioInterface);
                    usbDevice[index].usbInterface.ioInterface = NULL;
                }
                /* close the USB device interface */
                MACCAN_DEBUG_CODE(0, "close I/O device\n");
                (void) (*usbDevice[index].ioDevice)->USBDeviceClose(usbDevice[index].ioDevice);
                usbDevice[index].usbInterface.fOpened = false;
            }
            /* rest in pease */
            MACCAN_DEBUG_CODE(0, "release I/O device\n");
            (void) (*usbDevice[index].ioDevice)->Release(usbDevice[index].ioDevice);
            usbDevice[index].ioDevice = NULL;
            usbDevice[index].fPresent = false;
            MACCAN_DEBUG_CORE(" (R.I.P.)\n");
        }
        LEAVE_CRITICAL_SECTION(index);
        //MACCAN_DEBUG_FUNC("unlock\n");
        (void) pthread_mutex_destroy(&usbDevice[index].ptMutex);
    }
    (void) pthread_mutex_destroy(&usbDriver.ptMutex);
    fInitialized = false;
    return 0;
}

CANUSB_Handle_t CANUSB_OpenDevice(UInt16 vendorId, UInt16 productId, UInt8 channelNo) {
    CANUSB_Handle_t handle = CANUSB_INVALID_HANDLE;
    UInt8 channel = 0U;
    IOReturn kr;

    /* must be initialized */
    if (!fInitialized)
        return CANUSB_INVALID_HANDLE;
    
    /* look for the USB device in the device list */
    for (int index = 0; index < CANUSB_MAX_DEVICES; index++) {
        // FIXME: is a mutex required here?
        if (usbDevice[index].fPresent &&
            (usbDevice[index].u16VendorId == vendorId) &&
            (usbDevice[index].u16ProductId == productId)) {
            if (channel == channelNo) {
                handle = index;
                break;
            } else
                channel++;
        }
    }
    if (!IS_HANDLE_VALID(handle))
        return CANUSB_INVALID_HANDLE;
    
    /* open the USB device */
    MACCAN_DEBUG_FUNC("lock #%i\n", handle);
    ENTER_CRITICAL_SECTION(handle);
    if (usbDevice[handle].fPresent &&
        (usbDevice[handle].ioDevice != NULL)) {
        if (!usbDevice[handle].usbInterface.fOpened) {
            /* Open the device for exclusive access */
            kr = (*usbDevice[handle].ioDevice)->USBDeviceOpen(usbDevice[handle].ioDevice);
            if (kIOReturnSuccess != kr) {
                MACCAN_DEBUG_ERROR("+++ Unable to open device #%i: %08x\n", handle, kr);
                LEAVE_CRITICAL_SECTION(handle);
                return CANUSB_INVALID_HANDLE;
            }
            /* Configure the device */
            kr = ConfigureDevice(usbDevice[handle].ioDevice);
            if (kIOReturnSuccess != kr) {
                MACCAN_DEBUG_ERROR("+++ Unable to configure device #%i: %08x\n", handle, kr);
                (void) (*usbDevice[handle].ioDevice)->USBDeviceClose(usbDevice[handle].ioDevice);
                usbDevice[handle].usbInterface.fOpened = false;
                LEAVE_CRITICAL_SECTION(handle);
                return CANUSB_INVALID_HANDLE;
            }
            /* Get the interfaces */
            kr = FindInterface(usbDevice[handle].ioDevice, handle);
            if (kIOReturnSuccess != kr) {
                MACCAN_DEBUG_ERROR("+++ Unable to find interfaces on device #%i: %08x\n", handle, kr);
                (void) (*usbDevice[handle].ioDevice)->USBDeviceClose(usbDevice[handle].ioDevice);
                usbDevice[handle].usbInterface.fOpened = false;
                LEAVE_CRITICAL_SECTION(handle);
                return CANUSB_INVALID_HANDLE;
            }
            /* note: fOpened is true */
        } else {
            /* the USB interface is already opened */
            handle = CANUSB_INVALID_HANDLE;
        }
    } else {
        MACCAN_DEBUG_ERROR("+++ Unable to open a matching device (vendor = %03x, product = %03x, channel = %i)\n",
                     vendorId, productId, channelNo);
        handle = CANUSB_INVALID_HANDLE;
    }
    LEAVE_CRITICAL_SECTION(handle);
    MACCAN_DEBUG_FUNC("unlock\n");

    return handle;
}

CANUSB_Return_t CANUSB_CloseDevice(CANUSB_Handle_t handle) {
    IOReturn kr;
    int ret = 0;
    
    /* must be initialized */
    if (!fInitialized)
        return CANUSB_ERROR_NOTINIT;
    /* must be a valid handle */
    if (!IS_HANDLE_VALID(handle))
        return CANUSB_ERROR_HANDLE;
    
    /* close the USB device */
    MACCAN_DEBUG_FUNC("lock #%i\n", handle);
    ENTER_CRITICAL_SECTION(handle);
    if (usbDevice[handle].fPresent) {
        if (usbDevice[handle].usbInterface.fOpened) {
            /* close the USB interface interface(s) */
            if(usbDevice[handle].usbInterface.ioInterface) {
                MACCAN_DEBUG_CODE(0, "close and release I/O interface\n");
                kr = (*usbDevice[handle].usbInterface.ioInterface)->USBInterfaceClose(usbDevice[handle].usbInterface.ioInterface);
                if(kIOReturnSuccess != kr) {
                    MACCAN_DEBUG_ERROR("+++ Unable to close I/O interface of device #%i: %08x\n", handle, kr);
                    // TODO: how to handle this?
                }
                kr = (*usbDevice[handle].usbInterface.ioInterface)->Release(usbDevice[handle].usbInterface.ioInterface);
                if(kIOReturnSuccess != kr) {
                    MACCAN_DEBUG_ERROR("+++ Unable to release I/O interface of device #%i: %08x\n", handle, kr);
                    // TODO: how to handle this?
                }
                usbDevice[handle].usbInterface.ioInterface = NULL;
            }
            /* Close the task�s connection to the device */
            if (usbDevice[handle].ioDevice) {
                MACCAN_DEBUG_CODE(0, "close I/O device\n");
                kr = (*usbDevice[handle].ioDevice)->USBDeviceClose(usbDevice[handle].ioDevice);
                if(kIOReturnSuccess != kr) {
                    MACCAN_DEBUG_ERROR("+++ Unable to close I/O device #%i: %08x\n", handle, kr);
                    LEAVE_CRITICAL_SECTION(handle);
                    return CANUSB_ERROR_RESOURCE;
                }
            }
            usbDevice[handle].usbInterface.fOpened = false;
        } else {
            /* the USB interface is not opened */
            ret = CANUSB_ERROR_NOTINIT;
        }
    } else {
        /* the USB device is not available */
        ret = CANUSB_ERROR_HANDLE;
    }
    LEAVE_CRITICAL_SECTION(handle);
    MACCAN_DEBUG_FUNC("unlock\n");
    return ret;
}

CANUSB_Return_t CANUSB_DeviceRequest(CANUSB_Handle_t handle, CANUSB_SetupPacket_t setupPacket, void *buffer, UInt16 size, UInt32 *transferred)
{
    IOUSBDevRequest request;
    IOReturn kr;
    int ret = 0;
    
    /* must be initialized */
    if (!fInitialized)
        return CANUSB_ERROR_NOTINIT;
    /* must be a valid handle */
    if (!IS_HANDLE_VALID(handle))
        return CANUSB_ERROR_HANDLE;
    
    bzero(&request, sizeof(IOUSBDevRequest));
    request.bmRequestType = setupPacket.RequestType;
    request.bRequest = setupPacket.Request;
    request.wValue = setupPacket.Value;
    request.wIndex = setupPacket.Index;
    request.wLength = setupPacket.Length;
    request.pData = buffer;
    request.wLenDone = 0;
    (void) size;
    
    MACCAN_DEBUG_FUNC("lock #%i\n", handle);
    ENTER_CRITICAL_SECTION(handle);
    if (usbDevice[handle].fPresent &&
        (usbDevice[handle].ioDevice != NULL)) {
        kr = (*usbDevice[handle].ioDevice)->DeviceRequest(usbDevice[handle].ioDevice, &request);
        if (kIOReturnSuccess != kr) {
            MACCAN_DEBUG_ERROR("+++ Control transfer failed (%08x)\n", kr);
            LEAVE_CRITICAL_SECTION(handle);
            return CANUSB_ERROR_RESOURCE;
        }
        if (transferred)
            *transferred = request.wLenDone;
    } else {
        MACCAN_DEBUG_ERROR("+++ Sorry, device #%i is not available\n", handle);
        ret = CANUSB_ERROR_HANDLE;
    }
    LEAVE_CRITICAL_SECTION(handle);
    MACCAN_DEBUG_FUNC("unlock\n");
    return ret;
}

CANUSB_Return_t CANUSB_ReadPipe(CANUSB_Handle_t handle, UInt8 pipeRef, void *buffer, UInt32 *size) {
    IOReturn kr;
    int ret = 0;
    
    /* must be initialized */
    if (!fInitialized)
        return CANUSB_ERROR_NOTINIT;
    /* must be a valid handle */
    if (!IS_HANDLE_VALID(handle))
        return CANUSB_ERROR_HANDLE;
    /* check for NULL pointer */
    if (!buffer)
        return CANUSB_ERROR_NULLPTR;
    
    MACCAN_DEBUG_FUNC("lock #%i (%u)\n", handle, pipeRef);
    ENTER_CRITICAL_SECTION(handle);
    if (usbDevice[handle].fPresent &&
        (usbDevice[handle].usbInterface.fOpened) &&
        (usbDevice[handle].usbInterface.ioInterface != NULL)) {
        kr = (*usbDevice[handle].usbInterface.ioInterface)->ReadPipe(usbDevice[handle].usbInterface.ioInterface,
                                                                     pipeRef, buffer, size);
        if (kIOReturnSuccess != kr) {
            MACCAN_DEBUG_ERROR("+++ Unable to read pipe #%d (%08x)\n", pipeRef, kr);
            LEAVE_CRITICAL_SECTION(handle);
            return CANUSB_ERROR_RESOURCE;
        }
    } else {
        MACCAN_DEBUG_ERROR("+++ Sorry, device #%i is not opened or not available\n", handle);
        ret = !usbDevice[handle].fPresent ? CANUSB_ERROR_HANDLE : CANUSB_ERROR_NOTINIT;
    }
    LEAVE_CRITICAL_SECTION(handle);
    MACCAN_DEBUG_FUNC("unlock\n");
    return ret;
}

static void ReadPipeCallback(void *refCon, IOReturn result, void *arg0)
{
    CANUSB_UsbPipe_t *usbPipe = (CANUSB_UsbPipe_t *)refCon;
    UInt8 *buffer, index;
    UInt32 length = (UInt32)arg0;
    IOReturn kr;
    
    switch(result)
    {
    case kIOReturnSuccess:
        if (usbPipe) {
            if (!IS_HANDLE_VALID(usbPipe->handle))
                return;
            if (!usbDevice[usbPipe->handle].usbInterface.ioInterface)
                return;
            if (usbPipe->buffer.index >= 2)
                return;
            /* double-buffer strategy */
            index = usbPipe->buffer.index;
            buffer = usbPipe->buffer.data[index];
            usbPipe->buffer.index = index ? 0 : 1;
            /* preparation of the next asynchronous pipe read event (with our pipe context as reference, 6th argument) */
            kr = (*usbDevice[usbPipe->handle].usbInterface.ioInterface)->ReadPipeAsync(usbDevice[usbPipe->handle].usbInterface.ioInterface,
                                                                                       usbPipe->pipeRef,
                                                                                       usbPipe->buffer.data[usbPipe->buffer.index],
                                                                                       usbPipe->buffer.size,
                                                                                       ReadPipeCallback,
                                                                                       (void *)usbPipe);
            if (kIOReturnSuccess != kr) {
                MACCAN_DEBUG_ERROR("+++ Unable to read async pipe #%d of device #%d (%08x)\n", usbPipe->pipeRef, usbPipe->handle, kr);
                /* error: pipe is boken */
                usbPipe->running = false;
            }
            /* call the CALLBACK routine with the referenced pipe context */
            if (usbPipe->callback && length) {
                usbPipe->callback(usbPipe, buffer, length);
            }
        }
        break;
    case kIOReturnAborted:
        if (usbPipe) {
            MACCAN_DEBUG_CORE("!!! Aborted: read async pipe #%d of device #%d (%08x)\n", usbPipe->pipeRef, usbPipe->handle, result);
            usbPipe->running = false;
        } else {
            MACCAN_DEBUG_CORE("!!! Aborted: read async pipe #%d of device #%d (%08x)\n", 0, CANUSB_INVALID_HANDLE, result);
        }
        break;
    default:
        if (usbPipe) {
            MACCAN_DEBUG_ERROR("+++ Error: read async pipe #%d of device #%d (%08x)\n", usbPipe->pipeRef, usbPipe->handle, result);
            usbPipe->running = false;
        } else {
            MACCAN_DEBUG_ERROR("+++ Error: read async pipe #%d of device #%d (%08x)\n", 0, CANUSB_INVALID_HANDLE, result);
        }
        break;
    }
    return;
}

CANUSB_Return_t CANUSB_ReadPipeAsyncStart(CANUSB_Handle_t handle, UInt8 pipeRef, CANUSB_UsbPipe_t *usbPipe) {
    IOReturn kr;
    int ret = 0;
    
    /* must be initialized */
    if (!fInitialized)
        return CANUSB_ERROR_NOTINIT;
    /* must be a valid handle */
    if (!IS_HANDLE_VALID(handle))
        return CANUSB_ERROR_HANDLE;
    /* check for NULL pointer */
    if (!usbPipe ||
        !usbPipe->buffer.data[0] ||
        !usbPipe->buffer.data[1]  ||
        !usbPipe->msgQueue.queueElem ||
        !usbPipe->callback)
        return CANUSB_ERROR_NULLPTR;
    /* must not be running */
    if (usbPipe->running)
        return CANUSB_ERROR_RESOURCE;
    /* check handel and pipe reference */
    if ((usbPipe->handle != handle) ||
        (usbPipe->pipeRef != pipeRef))
        return CANUSB_ERROR_FATAL;
    
    MACCAN_DEBUG_FUNC("lock #%i (%u)\n", handle, pipeRef);
    ENTER_CRITICAL_SECTION(handle);
    if (usbDevice[handle].fPresent &&
        (usbDevice[handle].usbInterface.fOpened) &&
        (usbDevice[handle].usbInterface.ioInterface != NULL)) {
        /* preparation of the first asynchronous pipe read event (with out pipe context as reference, 6th argument) */
        kr = (*usbDevice[usbPipe->handle].usbInterface.ioInterface)->ReadPipeAsync(usbDevice[usbPipe->handle].usbInterface.ioInterface,
                                                                                   usbPipe->pipeRef,
                                                                                   usbPipe->buffer.data[usbPipe->buffer.index],
                                                                                   usbPipe->buffer.size,
                                                                                   ReadPipeCallback,
                                                                                   (void *)usbPipe);
        if (kIOReturnSuccess != kr) {
            MACCAN_DEBUG_ERROR("+++ Unable to start async pipe #%d of device #%d (%08x)\n", usbPipe->pipeRef, usbPipe->handle, kr);
            LEAVE_CRITICAL_SECTION(handle);
            return CANUSB_ERROR_RESOURCE;
        }
        /* asynchronous pipe read event armed */
        usbPipe->running = true;
    } else {
        MACCAN_DEBUG_ERROR("+++ Sorry, device #%i is not opened or not available\n", handle);
        ret = !usbDevice[handle].fPresent ? CANUSB_ERROR_HANDLE : CANUSB_ERROR_NOTINIT;
    }
    LEAVE_CRITICAL_SECTION(handle);
    MACCAN_DEBUG_FUNC("unlock\n");
    return ret;
}

CANUSB_Return_t CANUSB_ReadPipeAsyncAbort(CANUSB_Handle_t handle, UInt8 pipeRef) {
    IOReturn kr;
    int ret = 0;
    
    /* must be initialized */
    if (!fInitialized)
        return CANUSB_ERROR_NOTINIT;
    /* must be a valid handle */
    if (!IS_HANDLE_VALID(handle))
        return CANUSB_ERROR_HANDLE;
    
    MACCAN_DEBUG_FUNC("lock #%i (%u)\n", handle, pipeRef);
    ENTER_CRITICAL_SECTION(handle);
    if (usbDevice[handle].fPresent &&
        (usbDevice[handle].usbInterface.fOpened) &&
        (usbDevice[handle].usbInterface.ioInterface != NULL)) {
        kr = (*usbDevice[handle].usbInterface.ioInterface)->AbortPipe(usbDevice[handle].usbInterface.ioInterface,
                                                                      pipeRef);
        if (kIOReturnSuccess != kr) {
            MACCAN_DEBUG_ERROR("+++ Unable to abort async pipe #%d (%08x)\n", pipeRef, kr);
            LEAVE_CRITICAL_SECTION(handle);
            return CANUSB_ERROR_RESOURCE;
        }
    } else {
        MACCAN_DEBUG_ERROR("+++ Sorry, device #%i is not opened or not available\n", handle);
        ret = !usbDevice[handle].fPresent ? CANUSB_ERROR_HANDLE : CANUSB_ERROR_NOTINIT;
    }
    LEAVE_CRITICAL_SECTION(handle);
    MACCAN_DEBUG_FUNC("unlock\n");
    return ret;
}

CANUSB_Return_t CANUSB_WritePipe(CANUSB_Handle_t handle, UInt8 pipeRef, void *buffer, UInt32 size) {
    IOReturn kr;
    int ret = 0;
    
    /* must be initialized */
    if (!fInitialized)
        return CANUSB_ERROR_NOTINIT;
    /* must be a valid handle */
    if (!IS_HANDLE_VALID(handle))
        return CANUSB_ERROR_HANDLE;
    /* check for NULL pointer */
    if (!buffer)
        return CANUSB_ERROR_NULLPTR;
    
    MACCAN_DEBUG_FUNC("lock #%i (%u)\n", handle, pipeRef);
    ENTER_CRITICAL_SECTION(handle);
    if (usbDevice[handle].fPresent &&
        (usbDevice[handle].usbInterface.fOpened) &&
        (usbDevice[handle].usbInterface.ioInterface != NULL)) {
        kr = (*usbDevice[handle].usbInterface.ioInterface)->GetPipeStatus(usbDevice[handle].usbInterface.ioInterface,
                                                                          pipeRef);
        if (kIOReturnSuccess != kr) {
            MACCAN_DEBUG_ERROR("+++ Status of pipe #%d (%08x)\n", pipeRef, kr);
            LEAVE_CRITICAL_SECTION(handle);
            return CANUSB_ERROR_RESOURCE;
        }
        kr = (*usbDevice[handle].usbInterface.ioInterface)->WritePipe(usbDevice[handle].usbInterface.ioInterface,
                                                                      pipeRef, buffer, size);
        if (kIOReturnSuccess != kr) {
            MACCAN_DEBUG_ERROR("+++ Unable to write pipe #%d (%08x)\n", pipeRef, kr);
            LEAVE_CRITICAL_SECTION(handle);
            return CANUSB_ERROR_RESOURCE;
        }
    } else {
        MACCAN_DEBUG_ERROR("+++ Sorry, device #%i is not opened or not available\n", handle);
        ret = !usbDevice[handle].fPresent ? CANUSB_ERROR_HANDLE : CANUSB_ERROR_NOTINIT;
    }
    LEAVE_CRITICAL_SECTION(handle);
    MACCAN_DEBUG_FUNC("unlock\n");
    return ret;
}

CANUSB_Handle_t CANUSB_GetFirstDevice(void) {
    CANUSB_Handle_t handle = CANUSB_INVALID_HANDLE;

    /* must be initialized */
    if (!fInitialized)
        return CANUSB_INVALID_HANDLE;
    
    /* get the first registered device, if any */
    if (idxDevice != 0)
        idxDevice = 0;
    while (idxDevice < CANUSB_MAX_DEVICES) {
        if (usbDevice[idxDevice].fPresent &&
            (usbDevice[idxDevice].ioDevice != NULL)) {
            handle = idxDevice;
            break;
        }
        idxDevice++;
    }
    return handle;
}

CANUSB_Handle_t CANUSB_GetNextDevice(void) {
    CANUSB_Handle_t handle = CANUSB_INVALID_HANDLE;
    
    /* must be initialized */
    if (!fInitialized)
        return CANUSB_INVALID_HANDLE;
    
    /* get the next registered device, if any */
    if (idxDevice < CANUSB_MAX_DEVICES)
        idxDevice += 1;
    while (idxDevice < CANUSB_MAX_DEVICES) {
        if (usbDevice[idxDevice].fPresent &&
            (usbDevice[idxDevice].ioDevice != NULL)) {
            handle = idxDevice;
            break;
        }
        idxDevice++;
    }
    return handle;
}

Boolean CANUSB_IsDevicePresent(CANUSB_Handle_t handle) {
    Boolean ret = false;
    
    /* must be initialized */
    if (!fInitialized)
        return false;
    /* must be a valid handle */
    if (!IS_HANDLE_VALID(handle))
        return false;
    
    MACCAN_DEBUG_FUNC("lock #%i\n", handle);
    ENTER_CRITICAL_SECTION(handle);
    if (usbDevice[handle].fPresent &&
        (usbDevice[handle].ioDevice != NULL))
        ret = true;
    LEAVE_CRITICAL_SECTION(handle);
    MACCAN_DEBUG_FUNC("unlock\n");
    return ret;
}

Boolean CANUSB_IsDeviceOpened(CANUSB_Handle_t handle) {
    Boolean ret = false;
    
    /* must be initialized */
    if (!fInitialized)
        return false;
    /* must be a valid handle */
    if (!IS_HANDLE_VALID(handle))
        return false;
    
    MACCAN_DEBUG_FUNC("lock #%i\n", handle);
    ENTER_CRITICAL_SECTION(handle);
    if (usbDevice[handle].fPresent &&
        (usbDevice[handle].ioDevice != NULL) &&
        usbDevice[handle].usbInterface.fOpened &&
        (usbDevice[handle].usbInterface.ioInterface != NULL))
        ret = true;
    LEAVE_CRITICAL_SECTION(handle);
    MACCAN_DEBUG_FUNC("unlock\n");
    return ret;
}

CANUSB_Return_t CANUSB_GetDeviceName(CANUSB_Handle_t handle, char *buffer, size_t nbytes) {
    int ret = 0;
    
    /* must be initialized */
    if (!fInitialized)
        return CANUSB_ERROR_NOTINIT;
    /* must be a valid handle */
    if (!IS_HANDLE_VALID(handle))
        return CANUSB_ERROR_HANDLE;
    /* check for NULL pointer */
    if (!buffer)
        return CANUSB_ERROR_NULLPTR;
        
    MACCAN_DEBUG_FUNC("lock #%i\n", handle);
    ENTER_CRITICAL_SECTION(handle);
    if (usbDevice[handle].fPresent &&
        (usbDevice[handle].ioDevice != NULL)) {
        strncpy(buffer, usbDevice[handle].szName, nbytes);
    } else {
        MACCAN_DEBUG_ERROR("+++ Sorry, device #%i is not available\n", handle);
        ret = CANUSB_ERROR_HANDLE;
    }
    LEAVE_CRITICAL_SECTION(handle);
    MACCAN_DEBUG_FUNC("unlock\n");
    return ret;
}

CANUSB_Return_t CANUSB_GetDeviceVendorId(CANUSB_Handle_t handle, UInt16 *value) {
    int ret = 0;
    
    /* must be initialized */
    if (!fInitialized)
        return CANUSB_ERROR_NOTINIT;
    /* must be a valid handle */
    if (!IS_HANDLE_VALID(handle))
        return CANUSB_ERROR_HANDLE;
    /* check for NULL pointer */
    if (!value)
        return CANUSB_ERROR_NULLPTR;
        
    MACCAN_DEBUG_FUNC("lock #%i\n", handle);
    ENTER_CRITICAL_SECTION(handle);
    if (usbDevice[handle].fPresent &&
        (usbDevice[handle].ioDevice != NULL)) {
        *value = usbDevice[handle].u16VendorId;
    } else {
        MACCAN_DEBUG_ERROR("+++ Sorry, device #%i is not available\n", handle);
        ret = CANUSB_ERROR_HANDLE;
    }
    LEAVE_CRITICAL_SECTION(handle);
    MACCAN_DEBUG_FUNC("unlock\n");
    return ret;
}

CANUSB_Return_t CANUSB_GetDeviceProductId(CANUSB_Handle_t handle, UInt16 *value) {
    int ret = 0;
    
    /* must be initialized */
    if (!fInitialized)
        return CANUSB_ERROR_NOTINIT;
    /* must be a valid handle */
    if (!IS_HANDLE_VALID(handle))
        return CANUSB_ERROR_HANDLE;
    /* check for NULL pointer */
    if (!value)
        return CANUSB_ERROR_NULLPTR;
        
    MACCAN_DEBUG_FUNC("lock #%i\n", handle);
    ENTER_CRITICAL_SECTION(handle);
    if (usbDevice[handle].fPresent &&
        (usbDevice[handle].ioDevice != NULL)) {
        *value = usbDevice[handle].u16ProductId;
    } else {
        MACCAN_DEBUG_ERROR("+++ Sorry, device #%i is not available\n", handle);
        ret = CANUSB_ERROR_HANDLE;
    }
    LEAVE_CRITICAL_SECTION(handle);
    MACCAN_DEBUG_FUNC("unlock\n");
    return ret;
}

CANUSB_Return_t CANUSB_GetDeviceReleaseNo(CANUSB_Handle_t handle, UInt16 *value) {
    int ret = 0;
    
    /* must be initialized */
    if (!fInitialized)
        return CANUSB_ERROR_NOTINIT;
    /* must be a valid handle */
    if (!IS_HANDLE_VALID(handle))
        return CANUSB_ERROR_HANDLE;
    /* check for NULL pointer */
    if (!value)
        return CANUSB_ERROR_NULLPTR;
        
    MACCAN_DEBUG_FUNC("lock #%i\n", handle);
    ENTER_CRITICAL_SECTION(handle);
    if (usbDevice[handle].fPresent &&
        (usbDevice[handle].ioDevice != NULL)) {
        *value = usbDevice[handle].u16ReleaseNo;
    } else {
        MACCAN_DEBUG_ERROR("+++ Sorry, device #%i is not available\n", handle);
        ret = CANUSB_ERROR_HANDLE;
    }
    LEAVE_CRITICAL_SECTION(handle);
    MACCAN_DEBUG_FUNC("unlock\n");
    return ret;
}

CANUSB_Return_t CANUSB_GetDeviceLocation(CANUSB_Handle_t handle, UInt32 *value) {
    int ret = 0;
    
    /* must be initialized */
    if (!fInitialized)
        return CANUSB_ERROR_NOTINIT;
    /* must be a valid handle */
    if (!IS_HANDLE_VALID(handle))
        return CANUSB_ERROR_HANDLE;
    /* check for NULL pointer */
    if (!value)
        return CANUSB_ERROR_NULLPTR;
        
    MACCAN_DEBUG_FUNC("lock #%i\n", handle);
    ENTER_CRITICAL_SECTION(handle);
    if (usbDevice[handle].fPresent &&
        (usbDevice[handle].ioDevice != NULL)) {
        *value = usbDevice[handle].u32Location;
    } else {
        MACCAN_DEBUG_ERROR("+++ Sorry, device #%i is not available\n", handle);
        ret = CANUSB_ERROR_HANDLE;
    }
    LEAVE_CRITICAL_SECTION(handle);
    MACCAN_DEBUG_FUNC("unlock\n");
    return ret;
}

CANUSB_Return_t CANUSB_GetDeviceAddress(CANUSB_Handle_t handle, UInt16 *value) {
    int ret = 0;
    
    /* must be initialized */
    if (!fInitialized)
        return CANUSB_ERROR_NOTINIT;
    /* must be a valid handle */
    if (!IS_HANDLE_VALID(handle))
        return CANUSB_ERROR_HANDLE;
    /* check for NULL pointer */
    if (!value)
        return CANUSB_ERROR_NULLPTR;
        
    MACCAN_DEBUG_FUNC("lock #%i\n", handle);
    ENTER_CRITICAL_SECTION(handle);
    if (usbDevice[handle].fPresent &&
        (usbDevice[handle].ioDevice != NULL)) {
        *value = usbDevice[handle].u16Address;
    } else {
        MACCAN_DEBUG_ERROR("+++ Sorry, device #%i is not available\n", handle);
        ret = CANUSB_ERROR_HANDLE;
    }
    LEAVE_CRITICAL_SECTION(handle);
    MACCAN_DEBUG_FUNC("unlock\n");
    return ret;
}

CANUSB_Return_t CANUSB_GetInterfaceClass(CANUSB_Handle_t handle, UInt8 *value) {
    int ret = 0;
    
    /* must be initialized */
    if (!fInitialized)
        return CANUSB_ERROR_NOTINIT;
    /* must be a valid handle */
    if (!IS_HANDLE_VALID(handle))
        return CANUSB_ERROR_HANDLE;
    /* check for NULL pointer */
    if (!value)
        return CANUSB_ERROR_NULLPTR;
        
    MACCAN_DEBUG_FUNC("lock #%i\n", handle);
    ENTER_CRITICAL_SECTION(handle);
    if (usbDevice[handle].fPresent &&
        (usbDevice[handle].ioDevice != NULL) &&
        usbDevice[handle].usbInterface.fOpened &&
        (usbDevice[handle].usbInterface.ioInterface != NULL)) {
        *value = usbDevice[handle].usbInterface.u8Class;
    } else {
        MACCAN_DEBUG_ERROR("+++ Sorry, device #%i is not opened or not available\n", handle);
        ret = !usbDevice[handle].fPresent ? CANUSB_ERROR_HANDLE : CANUSB_ERROR_NOTINIT;
    }
    LEAVE_CRITICAL_SECTION(handle);
    MACCAN_DEBUG_FUNC("unlock\n");
    return ret;
}

CANUSB_Return_t CANUSB_GetInterfaceSubClass(CANUSB_Handle_t handle, UInt8 *value) {
    int ret = 0;
    
    /* must be initialized */
    if (!fInitialized)
        return CANUSB_ERROR_NOTINIT;
    /* must be a valid handle */
    if (!IS_HANDLE_VALID(handle))
        return CANUSB_ERROR_HANDLE;
    /* check for NULL pointer */
    if (!value)
        return CANUSB_ERROR_NULLPTR;
        
    MACCAN_DEBUG_FUNC("lock #%i\n", handle);
    ENTER_CRITICAL_SECTION(handle);
    if (usbDevice[handle].fPresent &&
        (usbDevice[handle].ioDevice != NULL) &&
        usbDevice[handle].usbInterface.fOpened &&
        (usbDevice[handle].usbInterface.ioInterface != NULL)) {
        *value = usbDevice[handle].usbInterface.u8SubClass;
    } else {
        MACCAN_DEBUG_ERROR("+++ Sorry, device #%i is not opened or not available\n", handle);
        ret = !usbDevice[handle].fPresent ? CANUSB_ERROR_HANDLE : CANUSB_ERROR_NOTINIT;
    }
    LEAVE_CRITICAL_SECTION(handle);
    MACCAN_DEBUG_FUNC("unlock\n");
    return ret;
}

CANUSB_Return_t CANUSB_GetInterfaceProtocol(CANUSB_Handle_t handle, UInt8 *value) {
    int ret = 0;
    
    /* must be initialized */
    if (!fInitialized)
        return CANUSB_ERROR_NOTINIT;
    /* must be a valid handle */
    if (!IS_HANDLE_VALID(handle))
        return CANUSB_ERROR_HANDLE;
    /* check for NULL pointer */
    if (!value)
        return CANUSB_ERROR_NULLPTR;
        
    MACCAN_DEBUG_FUNC("lock #%i\n", handle);
    ENTER_CRITICAL_SECTION(handle);
    if (usbDevice[handle].fPresent &&
        (usbDevice[handle].ioDevice != NULL) &&
        usbDevice[handle].usbInterface.fOpened &&
        (usbDevice[handle].usbInterface.ioInterface != NULL)) {
        *value = usbDevice[handle].usbInterface.u8Protocol;
    } else {
        MACCAN_DEBUG_ERROR("+++ Sorry, device #%i is not opened or not available\n", handle);
        ret = !usbDevice[handle].fPresent ? CANUSB_ERROR_HANDLE : CANUSB_ERROR_NOTINIT;
    }
    LEAVE_CRITICAL_SECTION(handle);
    MACCAN_DEBUG_FUNC("unlock\n");
    return ret;
}

CANUSB_Return_t CANUSB_GetInterfaceNumEndpoints(CANUSB_Handle_t handle, UInt8 *value) {
    int ret = 0;
    
    /* must be initialized */
    if (!fInitialized)
        return CANUSB_ERROR_NOTINIT;
    /* must be a valid handle */
    if (!IS_HANDLE_VALID(handle))
        return CANUSB_ERROR_HANDLE;
    /* check for NULL pointer */
    if (!value)
        return CANUSB_ERROR_NULLPTR;
        
    MACCAN_DEBUG_FUNC("lock #%i\n", handle);
    ENTER_CRITICAL_SECTION(handle);
    if (usbDevice[handle].fPresent &&
        (usbDevice[handle].ioDevice != NULL) &&
        usbDevice[handle].usbInterface.fOpened &&
        (usbDevice[handle].usbInterface.ioInterface != NULL)) {
        *value = usbDevice[handle].usbInterface.u8NumEndpoints;
    } else {
        MACCAN_DEBUG_ERROR("+++ Sorry, device #%i is not opened or not available\n", handle);
        ret = !usbDevice[handle].fPresent ? CANUSB_ERROR_HANDLE : CANUSB_ERROR_NOTINIT;
    }
    LEAVE_CRITICAL_SECTION(handle);
    MACCAN_DEBUG_FUNC("unlock\n");
    return ret;
}

UInt32 CANUSB_GetVersion(void) {
    return ((UInt32)VERSION_MAJOR << 24) |
           ((UInt32)VERSION_MINOR << 16) |
           ((UInt32)VERSION_PATCH << 8);
}

/*
 *  Working with USB Device Interfaces on macOS
 *  See https://developer.apple.com/library/archive/documentation/DeviceDrivers/Conceptual/USBBook
 *  Revision of 2012-01-09
 */
static int SetupDirectory(SInt32 vendorID, SInt32 productID)
{
    mach_port_t             masterPort;
    CFMutableDictionaryRef  matchingDict;
    CFNumberRef             numberRef;
    CFRunLoopSourceRef      runLoopSource;
    kern_return_t           kr;

    /* Create a master port for communication with the I/O Kit */
    kr = IOMasterPort(MACH_PORT_NULL, &masterPort);
    if (kr || !masterPort)
    {
        MACCAN_DEBUG_ERROR("+++ Couldn't create a master I/O Kit port (%08x)\n", kr);
        return CANUSB_ERROR_FATAL;
    }
    /* Set up matching dictionary for class IOUSBDevice and its subclasses */
    matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
    if (matchingDict == NULL)
    {
        MACCAN_DEBUG_ERROR("+++ Couldn't create a USB matching dictionary\n");
        mach_port_deallocate(mach_task_self(), masterPort);
        return CANUSB_ERROR_FATAL;
    }
    /* Add the vendor and product IDs to the matching dictionary */
    numberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &vendorID);
    CFDictionarySetValue(matchingDict,  CFSTR(kUSBVendorID), numberRef);
    CFRelease(numberRef);
    numberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &productID);
    CFDictionarySetValue(matchingDict, CFSTR(kUSBProductID), numberRef);
    CFRelease(numberRef);
    numberRef = NULL;

    usbDriver.refRunLoop = CFRunLoopGetCurrent();
    //To set up asynchronous notifications, create a notification port and
    //add its run loop event source to the program�s run loop
    usbDriver.refNotifyPort = IONotificationPortCreate(masterPort);
    runLoopSource = IONotificationPortGetRunLoopSource(usbDriver.refNotifyPort);
    CFRunLoopAddSource(usbDriver.refRunLoop, runLoopSource,
                      kCFRunLoopDefaultMode);
    //Retain additional dictionary references because each call to
    //IOServiceAddMatchingNotification consumes one reference
    matchingDict = (CFMutableDictionaryRef) CFRetain(matchingDict);
    matchingDict = (CFMutableDictionaryRef) CFRetain(matchingDict);

    //Now set up two notifications: one to be called when a bulk test device
    //is first matched by the I/O Kit and another to be called when the
    //device is terminated.
    //Notification of first match
    (void) IOServiceAddMatchingNotification(usbDriver.refNotifyPort,
                  kIOFirstMatchNotification, matchingDict,
                  DeviceAdded, NULL, &usbDriver.iterBulkDevicePlugged);
    //Iterate over set of matching devices to access already-present devices
    //and to arm the notification
    DeviceAdded(NULL, usbDriver.iterBulkDevicePlugged);
    //Notification of termination
    (void) IOServiceAddMatchingNotification(usbDriver.refNotifyPort,
                  kIOTerminatedNotification, matchingDict,
                  DeviceRemoved, NULL, &usbDriver.iterBulkDeviceUnplugged);
    //Iterate over set of matching devices to release each one and to
    //arm the notification. NOTE: this function is not shown in this document.
    DeviceRemoved(NULL, usbDriver.iterBulkDeviceUnplugged);
    /* Finished with master port */
    mach_port_deallocate(mach_task_self(), masterPort);
    masterPort = 0;
    return 0;
}

static void DeviceAdded(void *refCon, io_iterator_t iterator)
{
    io_service_t            service;
    IOCFPlugInInterface     **plugInInterface = NULL;
    IOUSBDeviceInterface    **device=NULL;
    HRESULT                 result;
    SInt32                  score;
    UInt16                  vendor;
    UInt16                  product;
    UInt16                  release;
    UInt32                  location;
    USBDeviceAddress        address;
    UInt8                   speed;
    io_name_t               name;
    kern_return_t           kr;
    int index, found;

    while((service = IOIteratorNext(iterator)))
    {
        /* Get the USB device's name */
        kr = IORegistryEntryGetName(service, name);
        if (KERN_SUCCESS != kr) {
            name[0] = '\0';
        }
        /* Create an intermediate plug-in using the IOCreatePlugInInterfaceForService function */
        (void) IOCreatePlugInInterfaceForService(service, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plugInInterface, &score);
        /* Release the device object after getting the intermediate plug-in */
        kr = IOObjectRelease(service);
        if((kIOReturnSuccess != kr) || !plugInInterface)
        {
            MACCAN_DEBUG_ERROR("+++ IOCreatePlugInInterfaceForService returned 0x%08x.\n", kr);
            continue;
        }
        /* Create the device interface using the QueryInterface function */
        result = (*plugInInterface)->QueryInterface(plugInInterface, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID), (LPVOID *)&device);
        /* Release the intermediate plug-in object */
        (*plugInInterface)->Release(plugInInterface);
        if (result || !device)
        {
            MACCAN_DEBUG_ERROR("+++ Couldn't create a device interface (%08x).\n", (int)result);
            continue;
        }
        /* Check the vendor, product, and release number values to confirm we�ve got the right device */
        (void) (*device)->GetDeviceVendor(device, &vendor);
        (void) (*device)->GetDeviceProduct(device, &product);
        (void) (*device)->GetDeviceReleaseNumber(device, &release);
        (void) (*device)->GetLocationID(device, &location);
        (void) (*device)->GetDeviceAddress(device, &address);
        (void) (*device)->GetDeviceSpeed(device, &speed);
        if (!CANDEV_GetDeviceById(vendor, product)) {
            MACCAN_DEBUG_ERROR("+++ Found unwanted device (vendor = %03x, product = %03x)\n", vendor, product);
            (void) (*device)->Release(device);
            continue;
        }
        MACCAN_DEBUG_CORE("    - One device added at location %08x\n", location);

        /* look for a free entry in the device list */
        for (index = 0, found = 0; (index < CANUSB_MAX_DEVICES) && !found; index++) {
            ENTER_CRITICAL_SECTION(index);
            if(!usbDevice[index].fPresent) {
                MACCAN_DEBUG_CORE("      - Device #%i: %s\n", index, name);
                MACCAN_DEBUG_CORE("        - Properties: vendor = %03x, product = %03x, release = %04x, speed = %d\n",
                                             vendor, product, release, speed);
                /* store the properties of the added device */
                bzero(&usbDevice[index].usbInterface, sizeof(USBInterface_t));
                usbDevice[index].usbInterface.fOpened = false;
                strcpy(usbDevice[index].szName, name);
                usbDevice[index].u16VendorId = vendor;
                usbDevice[index].u16ProductId = product;
                usbDevice[index].u16ReleaseNo = release;
                usbDevice[index].u32Location = location;
                usbDevice[index].u16Address = address;
                usbDevice[index].ioDevice = device;
                usbDevice[index].fPresent = true;
                found = 1;
            }
            LEAVE_CRITICAL_SECTION(index);
        }
        if (!found) {
            /* no free entry available */
            MACCAN_DEBUG_ERROR("+++ No free entry available for new device (vendor = %03x, product = %03x)\n", vendor, product);
            (void) (*device)->Release(device);
        }
    }
    (void)refCon;  /* to avoid warnings */
}

static void DeviceRemoved(void *refCon, io_iterator_t iterator)
{
    kern_return_t   kr;
    io_service_t    object;
    UInt64          location;
    CFTypeRef       locationCF;
    int index;

    while((object = IOIteratorNext(iterator)))
    {
        /* Get the location from the i/o registry */
        locationCF = IORegistryEntryCreateCFProperty (object, CFSTR(kUSBDevicePropertyLocationID), kCFAllocatorDefault, 0);
        CFNumberGetValue(locationCF, kCFNumberLongType, (void *)&location);
        CFRelease (locationCF);

        /* Release the device object */
        kr = IOObjectRelease(object);
        if (kr != kIOReturnSuccess)
        {
            MACCAN_DEBUG_ERROR("+++ Couldn't release device object: %08x\n", kr);
            continue;
        }
        MACCAN_DEBUG_CORE("    - One device removed from location %08x\n", location);

        /* remove the device from the device list */
        for (index = 0; index < CANUSB_MAX_DEVICES; index++) {
            ENTER_CRITICAL_SECTION(index);
            if (location == (UInt64)usbDevice[index].u32Location) {
                MACCAN_DEBUG_CORE("      - Device #%i is %s available (vendor = %03x, product = %03x)\n", index,
                             usbDevice[index].fPresent? "not longer" : "not", usbDevice[index].u16VendorId, usbDevice[index].u16ProductId);
                /* reset the properties of the removed device */
                bzero(&usbDevice[index].usbInterface, sizeof(USBInterface_t));
                usbDevice[index].usbInterface.fOpened = false;
                usbDevice[index].u16VendorId = 0x0U;
                usbDevice[index].u16ProductId = 0x0U;
                usbDevice[index].u16ReleaseNo = 0x0U;
                usbDevice[index].u32Location = 0x0U;
                usbDevice[index].u16Address = 0x0U;
                usbDevice[index].ioDevice = NULL;
                usbDevice[index].fPresent = false;
            }
            LEAVE_CRITICAL_SECTION(index);
        }
    }
    (void)refCon;  /* to avoid warnings */
}

static IOReturn ConfigureDevice(IOUSBDeviceInterface **dev)
{
    UInt8                           numConfig;
    IOReturn                        kr;
    IOUSBConfigurationDescriptorPtr configDesc;

    //Get the number of configurations. The sample code always chooses
    //the first configuration (at index 0) but your code may need a
    //different one
    kr = (*dev)->GetNumberOfConfigurations(dev, &numConfig);
    if (kr || !numConfig)
        return -1;
    //Get the configuration descriptor for index 0
    kr = (*dev)->GetConfigurationDescriptorPtr(dev, 0, &configDesc);
    if (kr)
    {
        MACCAN_DEBUG_ERROR("+++ Couldn't get configuration descriptor for index %d (err = %08x)\n", 0, kr);
        return kr;
    }
    //Set the device�s configuration. The configuration value is found in
    //the bConfigurationValue field of the configuration descriptor
    kr = (*dev)->SetConfiguration(dev, configDesc->bConfigurationValue);
    if (kr)
    {
        MACCAN_DEBUG_ERROR("+++ Couldn't set configuration to value %d (err = %08x)\n", 0, kr);
        return kr;
    }
    return kIOReturnSuccess;
}

static IOReturn FindInterface(IOUSBDeviceInterface **device, int index)
{
    IOReturn                    kr=0;
    IOUSBFindInterfaceRequest   request;
    io_iterator_t               iterator;
    io_service_t                usbInterface;
    IOCFPlugInInterface         **plugInInterface = NULL;
    IOUSBInterfaceInterface     **interface = NULL;
    HRESULT                     result;
    SInt32                      score;
    UInt8                       interfaceClass;
    UInt8                       interfaceSubClass;
    UInt8                       interfaceProtocol;
    UInt8                       interfaceNumEndpoints;
    CFRunLoopSourceRef          runLoopSource;
#ifdef PRINT_USB_PIPE_INFO
    int                         pipeRef;
#endif

    /* Allow to find all the interfaces */
    request.bInterfaceClass = kIOUSBFindInterfaceDontCare;
    request.bInterfaceSubClass = kIOUSBFindInterfaceDontCare;
    request.bInterfaceProtocol = kIOUSBFindInterfaceDontCare;
    request.bAlternateSetting = kIOUSBFindInterfaceDontCare;
    /* Get an iterator for the interfaces on the device */
    kr = (*device)->CreateInterfaceIterator(device, &request, &iterator);
    if(kIOReturnSuccess != kr)
    {
        MACCAN_DEBUG_ERROR("+++ Unable to create interface iterator (%08x)\n", kr);
        return kr;
    }
    kr = kIOReturnError;
    while((usbInterface = IOIteratorNext(iterator)))
    {
        /* Create an intermediate plug-in */
        (void) IOCreatePlugInInterfaceForService(usbInterface, kIOUSBInterfaceUserClientTypeID, kIOCFPlugInInterfaceID, &plugInInterface, &score);
        /* Release the usbInterface object after getting the plug-in */
        kr = IOObjectRelease(usbInterface);
        if((kIOReturnSuccess != kr) || !plugInInterface)
        {
            MACCAN_DEBUG_ERROR("+++ Unable to create a plug-in (%08x)\n", kr);
            break;
        }
        /* Now create the device interface for the interface */
        result = (*plugInInterface)->QueryInterface(plugInInterface, CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID), (LPVOID *) &interface);
        /* No longer need the intermediate plug-in */
        (*plugInInterface)->Release(plugInInterface);
        if (result || !interface)
        {
            MACCAN_DEBUG_ERROR("+++ Couldn't create a device interface for the interface (%08x)\n", (int) result);
            break;
        }
        /* Get interface class and subclass */
        (void) (*interface)->GetInterfaceClass(interface, &interfaceClass);
        (void) (*interface)->GetInterfaceSubClass(interface, &interfaceSubClass);
        (void) (*interface)->GetInterfaceProtocol(interface, &interfaceProtocol);
        /* Now open the interface. This will cause the pipes associated with */
        /* the endpoints in the interface descriptor to be instantiated */
        kr = (*interface)->USBInterfaceOpen(interface);
        if(kIOReturnSuccess != kr)
        {
            MACCAN_DEBUG_ERROR("+++ Unable to open interface (%08x)\n", kr);
            (void) (*interface)->Release(interface);
            break;
        }
        /* Get the number of endpoints associated with this interface */
        kr = (*interface)->GetNumEndpoints(interface, &interfaceNumEndpoints);
        if(kIOReturnSuccess != kr)
        {
            MACCAN_DEBUG_ERROR("+++ Unable to get number of endpoints (%08x)\n", kr);
            (void) (*interface)->USBInterfaceClose(interface);
            (void) (*interface)->Release(interface);
            break;
        }
#ifdef PRINT_USB_PIPE_INFO
        MACCAN_DEBUG_CORE("      - Interface class %d, subclass %d, protocol %d\n", interfaceClass, interfaceSubClass, interfaceProtocol);
        MACCAN_DEBUG_CORE("      - Interface has %d endpoints:\n", interfaceNumEndpoints);
        /* Access each pipe in turn, starting with the pipe at index 1 */
        /* The pipe at index 0 is the default control pipe and should */
        /* be accessed using (*usbDevice)->DeviceRequest() instead */
        for(pipeRef = 1; pipeRef <= interfaceNumEndpoints; pipeRef++)
        {
            IOReturn        kr2;
            UInt8           direction;
            UInt8           number;
            UInt8           transferType;
            UInt16          maxPacketSize;
            UInt8           interval;
            char            *message;

            kr2 = (*interface)->GetPipeProperties(interface, pipeRef, &direction, &number, &transferType, &maxPacketSize, &interval);
            if(kIOReturnSuccess != kr2)
                MACCAN_DEBUG_ERROR("+++ Unable to get properties of pipe #%d (%08x)\n", pipeRef, kr2);
            else
            {
                MACCAN_DEBUG_CORE("          - PipeRef #%d: ", pipeRef);
                switch (direction)
                {
                    case kUSBOut:     message = "out"; break;
                    case kUSBIn:      message = "in"; break;
                    case kUSBNone:    message = "none"; break;
                    case kUSBAnyDirn: message = "any"; break;
                    default:          message = "???"; break;
                }
                MACCAN_DEBUG_CORE("direction %s, ", message);
                switch (transferType)
                {
                    case kUSBControl:   message = "control"; break;
                    case kUSBIsoc:      message = "isoc"; break;
                    case kUSBBulk:      message = "bulk"; break;
                    case kUSBInterrupt: message = "interrupt"; break;
                    case kUSBAnyType:   message = "any"; break;
                    default:            message = "???"; break;
                }
                MACCAN_DEBUG_CORE("transfer type %s, max. packet size %d\n", message, maxPacketSize);
            }
        }
#endif
        /* For this test, just use first interface, so exit loop */
        if (IS_HANDLE_VALID(index)) {
            usbDevice[index].usbInterface.hDevice = index;
            usbDevice[index].usbInterface.ioInterface = interface;
            usbDevice[index].usbInterface.u8Class = interfaceClass;
            usbDevice[index].usbInterface.u8SubClass = interfaceSubClass;
            usbDevice[index].usbInterface.u8Protocol = interfaceProtocol;
            usbDevice[index].usbInterface.u8NumEndpoints = interfaceNumEndpoints;
            /* As with service matching notifications, to receive asynchronous */
            /* I/O completion notifications, you must create an event source and */
            /* add it to the run loop */
            kr = (*interface)->CreateInterfaceAsyncEventSource(
                                    interface, &runLoopSource);
            if (kr != kIOReturnSuccess)
            {
                MACCAN_DEBUG_ERROR("+++ Unable to create asynchronous event source for device #%i (%08x)\n", index, kr);
                (void) (*interface)->USBInterfaceClose(interface);
                (void) (*interface)->Release(interface);
                break;
            }
            CFRunLoopAddSource(usbDriver.refRunLoop, runLoopSource,
                                    kCFRunLoopDefaultMode);
            MACCAN_DEBUG_CORE("      + Device #%i: asynchronous event source added to run loop\n", index);
            /* the USB interface can now be used */
            usbDevice[index].usbInterface.fOpened = true;
            kr = kIOReturnSuccess;
        }
        else
        {
            (void) (*interface)->USBInterfaceClose(interface);
            (void) (*interface)->Release(interface);
            kr = kIOReturnError;
        }
        break;
    }
    /* Clean up used resources */
    (void) IOObjectRelease(iterator);
    return kr;
}

static void* WorkerThread(void* arg)
{
    const CANDEV_Device_t *ptrDevice = CANDEV_GetFirstDevice();

    /* set up the IOUSBKit to manage and access CAN to USB devices */
    while (ptrDevice) {
        if (SetupDirectory((SInt32)ptrDevice->vendorId,
                           (SInt32)ptrDevice->productId) != 0)
            goto exit_worker_thread;
        ptrDevice = CANDEV_GetNextDevice();
    }
    /* indicate to the creator that the thread is running */
    assert(0 == pthread_mutex_lock(&usbDriver.ptMutex));
    usbDriver.fRunning = TRUE;
    assert(0 == pthread_mutex_unlock(&usbDriver.ptMutex));

    /* start the run loop so notifications will be received */
    MACCAN_DEBUG_CORE("    - Run loop started so notifications will be received\n");
    CFRunLoopRun();

    /* fatal error: execution should never come here! */
    MACCAN_DEBUG_ERROR("+++ Oops, something went terribly wrong!\a\n");
    assert(0 == pthread_mutex_lock(&usbDriver.ptMutex));
    usbDriver.fRunning = FALSE;
    assert(0 == pthread_mutex_unlock(&usbDriver.ptMutex));

    (void)arg;  /* to avoid compiler warnings */
exit_worker_thread:
    /* terminate the thread */
    pthread_exit(NULL);
    return NULL;
}

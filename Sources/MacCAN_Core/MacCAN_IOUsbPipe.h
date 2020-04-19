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
#ifndef MACCAN_IOUSBPIPE_H_INCLUDED
#define MACCAN_IOUSBPIPE_H_INCLUDED

#include "MacCAN_IOUsbKit.h"

/* CAN API V3 compatible error codes */
#define CANUSB_ERROR_OK  (0)
#define CANUSB_ERROR_FULL  (-20)
#define CANUSB_ERROR_EMPTY  (-30)
#define CANUSB_ERROR_RESOURCE  (-90)
#define CANUSB_ERROR_FATAL  (-99)

/* CAN API V3 compatible time-out value */
#define CANUSB_INFINITE  (65535U)

/* macro to set up a MacCAN pipe context */
#define CANUSB_PIPE_CONTEXT(ptr,hnd,ref,cbk,para) \
                            do{ if (ptr) { \
                                    ptr->handle = hnd; \
                                    ptr->pipeRef = ref; \
                                    ptr->callback = cbk; \
                                    ptr->ptrParam = para; \
                            } } while(0)
#define CANUSB_PIPE_RUNNING(pipe)  pipe.running
#define CANUSB_QUEUE_OVERFLOW(pipe)  pipe.msgQueue.ovfl.flag

#ifdef __cplusplus
extern "C" {
#endif

extern CANUSB_Return_t CANUSB_CreatePipe(CANUSB_UsbPipe_t *usbPipe, size_t bufferSize, size_t numElem, size_t elemSize);

extern CANUSB_Return_t CANUSB_DestroyPipe(CANUSB_UsbPipe_t *usbPipe);

extern CANUSB_Return_t CANUSB_Enqueue(CANUSB_UsbPipe_t *usbPipe, void const *message/*, UInt16 timeout*/);

extern CANUSB_Return_t CANUSB_Dequeue(CANUSB_UsbPipe_t *usbPipe, void *message, UInt16 timeout);

extern CANUSB_Return_t CANUSB_ResetQueue(CANUSB_UsbPipe_t *usbPipe);

// TODO: (1) transmission queue (the wait event is on the opposite side)
// TODO: (2) Ceci n'est pas une pipe (to have a file descriptor for PCBUSB)

#ifdef __cplusplus
}
#endif
#endif /* MACCAN_IOUSBPIPE_H_INCLUDED */


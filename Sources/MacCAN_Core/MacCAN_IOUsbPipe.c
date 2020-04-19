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
#include "MacCAN_IOUsbPipe.h"
#include "MacCAN_Debug.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define ENTER_CRITICAL_SECTION(queue)  assert(0 == pthread_mutex_lock(&queue->wait.mutex))
#define LEAVE_CRITICAL_SECTION(queue)  assert(0 == pthread_mutex_unlock(&queue->wait.mutex))

#define GET_TIME(ts)  do{ clock_gettime(CLOCK_REALTIME, &ts); } while(0)
#define ADD_TIME(ts,to)  do{ ts.tv_sec += (time_t)(to / 1000U); \
                             ts.tv_nsec += (long)(to % 1000U) * (long)1000000; \
                             if (ts.tv_nsec >= (long)1000000000) { \
                                 ts.tv_nsec %= (long)1000000000; \
                                 ts.tv_sec += (time_t)1; \
                             } } while(0)

#define SIGNAL_WAIT_CONDITION(queue)  do{ msgQueue->wait.flag = true; \
                                          assert(0 == pthread_cond_signal(&queue->wait.cond)); } while(0)
#define WAIT_CONDITION_INFINITE(queue,res)  do{ queue->wait.flag = false; \
                                                res = pthread_cond_wait(&queue->wait.cond, &queue->wait.mutex); } while(0)
#define WAIT_CONDITION_TIMEOUT(queue,abstime,res)  do{ queue->wait.flag = false; \
                                                       res = pthread_cond_timedwait(&queue->wait.cond, &queue->wait.mutex, &abstime); } while(0)
 
static Boolean EnqueueElement(CANUSB_MsgQueue_t *queue, const void *element);
static Boolean DequeueElement(CANUSB_MsgQueue_t *queue, void *element);

CANUSB_Return_t CANUSB_CreatePipe(CANUSB_UsbPipe_t *usbPipe, size_t bufferSize, size_t numElem, size_t elemSize) {
    CANUSB_Return_t retVal = CANUSB_ERROR_RESOURCE;
    
    if (usbPipe) {
        bzero(usbPipe, sizeof(CANUSB_UsbPipe_t));
        /* create a double buffer for USB data transfer */
        MACCAN_DEBUG_DRIVER("        - Double buffer each of size %i bytes\n", bufferSize);
        if ((usbPipe->buffer.data[0] = malloc(bufferSize)) &&
            (usbPipe->buffer.data[1] = malloc(bufferSize))) {
            usbPipe->buffer.size = (UInt32)bufferSize;
            retVal = CANUSB_ERROR_OK;
        } else {
            MACCAN_DEBUG_ERROR("+++ Unable to create double buffer (2 * %i bytes)\n", bufferSize);
        }
        /* create a message queue for data exchange */
        MACCAN_DEBUG_DRIVER("        - Message queue for %i elements of size %i bytes\n", numElem, elemSize);
        bzero(&usbPipe->msgQueue, sizeof(CANUSB_MsgQueue_t));
        if ((usbPipe->msgQueue.queueElem = calloc(numElem, elemSize))) {
            if ((pthread_mutex_init(&usbPipe->msgQueue.wait.mutex, NULL) == 0) &&
                (pthread_cond_init(&usbPipe->msgQueue.wait.cond, NULL)) == 0) {
                usbPipe->msgQueue.elemSize = (size_t)elemSize;
                usbPipe->msgQueue.size = (UInt32)numElem;
                usbPipe->msgQueue.wait.flag = false;
                usbPipe->running = false;
                retVal = CANUSB_ERROR_OK;
            }
        } else {
            MACCAN_DEBUG_ERROR("+++ Unable to create message queue (%i * %i bytes)\n", numElem, elemSize);
        }
    } else {
        MACCAN_DEBUG_ERROR("+++ Invalid pipe context given (NULL pointer)");
    }
    return retVal;
}

CANUSB_Return_t CANUSB_DestroyPipe(CANUSB_UsbPipe_t *usbPipe) {
    CANUSB_Return_t retVal =  CANUSB_ERROR_RESOURCE;
    
    if (usbPipe) {
        pthread_cond_destroy(&usbPipe->msgQueue.wait.cond);
        pthread_mutex_destroy(&usbPipe->msgQueue.wait.mutex);
        if (usbPipe->msgQueue.queueElem)
            free(usbPipe->msgQueue.queueElem);
        bzero(&usbPipe->msgQueue, sizeof(CANUSB_MsgQueue_t));
        if (usbPipe->buffer.data[1])
            free(usbPipe->buffer.data[1]);
        if (usbPipe->buffer.data[0])
            free(usbPipe->buffer.data[0]);
        bzero(&usbPipe->buffer, sizeof(CANUSB_Buffer_t));
        usbPipe->callback = NULL;
        /* note: an abort request can still pending */
        retVal = CANUSB_ERROR_OK;
    } else {
        MACCAN_DEBUG_ERROR("+++ Invalid pipe context given (NULL pointer)");
    }
    return retVal;
}

CANUSB_Return_t CANUSB_Enqueue(CANUSB_UsbPipe_t *usbPipe, void const *message/*, UInt16 timeout*/) {
    CANUSB_MsgQueue_t *msgQueue = (usbPipe) ? &usbPipe->msgQueue : NULL;
    CANUSB_Return_t retVal = CANUSB_ERROR_RESOURCE;

    if (message && msgQueue) {
        ENTER_CRITICAL_SECTION(msgQueue);
        if (EnqueueElement(msgQueue, message)) {
            SIGNAL_WAIT_CONDITION(msgQueue);
            retVal = CANUSB_ERROR_OK;
        } else {
            retVal = CANUSB_ERROR_FULL;
        }
        LEAVE_CRITICAL_SECTION(msgQueue);
    } else {
        MACCAN_DEBUG_ERROR("+++ Couldn't enqueue message (NULL pointer)");
    }
    return retVal;
}

CANUSB_Return_t CANUSB_Dequeue(CANUSB_UsbPipe_t *usbPipe, void *message, UInt16 timeout) {
    CANUSB_MsgQueue_t *msgQueue = (usbPipe) ? &usbPipe->msgQueue : NULL;
    CANUSB_Return_t retVal = CANUSB_ERROR_RESOURCE;
    struct timespec absTime;
    int waitCond = 0;

    GET_TIME(absTime);
    ADD_TIME(absTime, timeout);

    if (message && msgQueue) {
        ENTER_CRITICAL_SECTION(msgQueue);
dequeue:
        if (DequeueElement(msgQueue, message)) {
            retVal = CANUSB_ERROR_OK;
        } else {
            if (timeout == CANUSB_INFINITE) {  /* blocking read */
                WAIT_CONDITION_INFINITE(msgQueue, waitCond);
                if ((waitCond == 0) && msgQueue->wait.flag)
                    goto dequeue;
            } else if (timeout != 0U) {  /* timed blocking read */
                WAIT_CONDITION_TIMEOUT(msgQueue, absTime, waitCond);
                if ((waitCond == 0) && msgQueue->wait.flag)
                    goto dequeue;
            }
            retVal = CANUSB_ERROR_EMPTY;
        }
        LEAVE_CRITICAL_SECTION(msgQueue);
    } else {
        MACCAN_DEBUG_ERROR("+++ Couldn't enqueue message (NULL pointer)");
    }
    return retVal;
}

CANUSB_Return_t CANUSB_ResetQueue(CANUSB_UsbPipe_t *usbPipe) {
    CANUSB_MsgQueue_t *msgQueue = (usbPipe) ? &usbPipe->msgQueue : NULL;
    CANUSB_Return_t retVal = CANUSB_ERROR_RESOURCE;

    if (msgQueue) {
        ENTER_CRITICAL_SECTION(msgQueue);
        msgQueue->used = 0U;
        msgQueue->head = 0U;
        msgQueue->tail = 0U;
        msgQueue->wait.flag = false;
        msgQueue->ovfl.flag = false;
        msgQueue->ovfl.counter = 0U;
        LEAVE_CRITICAL_SECTION(msgQueue);
        retVal = CANUSB_ERROR_OK;
    } else {
        MACCAN_DEBUG_ERROR("+++ Couldn't reset queue (NULL pointer)");
    }
    return retVal;
}

/*  ---  FIFO  ---
 *
 *  size :  total number of elements
 *  head :  read position of the queue
 *  tail :  write position of the queue
 *  used :  number of queued elements
 *
 *  (§1) empty :  used == 0
 *  (§2) full  :  used == size  &&  size > 0
 */
static Boolean EnqueueElement(CANUSB_MsgQueue_t *queue, const void *element) {
    assert(queue);
    assert(element);
    assert(queue->size);
    assert(queue->queueElem);
    
    if (queue->used < queue->size) {
        if (queue->used != 0U)
            queue->tail = (queue->tail + 1U) % queue->size;
        else
            queue->head = queue->tail;  /* to make sure */
        (void) memcpy(&queue->queueElem[(queue->tail * queue->elemSize)], element, queue->elemSize);
        queue->used += 1U;
        return true;
    } else {
        queue->ovfl.counter += 1U;
        queue->ovfl.flag = true;
        return false;
    }
}

static Boolean DequeueElement(CANUSB_MsgQueue_t *queue, void *element) {
    assert(queue);
    assert(element);
    assert(queue->size);
    assert(queue->queueElem);

    if (queue->used > 0U) {
        (void) memcpy(element, &queue->queueElem[(queue->head * queue->elemSize)], queue->elemSize);
        queue->head = (queue->head + 1U) % queue->size;
        queue->used -= 1U;
        return true;
    } else
        return false;
}


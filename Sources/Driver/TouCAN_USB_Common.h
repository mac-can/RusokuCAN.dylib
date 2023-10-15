/*  SPDX-License-Identifier: GPL-3.0-or-later */
/*
 *  TouCAN - macOS User-Space Driver for Rusoku TouCAN USB Interfaces
 *
 *  Copyright (C) 2021-2023  Uwe Vogt, UV Software, Berlin (info@mac-can.com)
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
#ifndef TOUCAN_USB_COMMON_H_INCLUDED
#define TOUCAN_USB_COMMON_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>

#define TOUCAN_VENDOR_ID  (UInt16)0x16D0
#define TOUCAN_USB_ID     (UInt16)0x0EAC

#define TOUCAN_VENDOR_NAME  "Rusoku technologijos UAB, Lithuania"
#define TOUCAN_VENDOR_URL   "www.rusoku.com"

// TODO: move this into USB_Driver.h (when MacCAN enpoind module realized)
#define TOUCAN_USB_TX_DATA_PIPE_REF  1U
#define TOUCAN_USB_RX_DATA_PIPE_REF  2U
#define TOUCAN_USB_TX_DATA_PIPE_SIZE  64U
#define TOUCAN_USB_RX_DATA_PIPE_SIZE  64U
#define TOUCAN_USB_TX_DATA_FRAME_SIZE  18U
#define TOUCAN_USB_RX_DATA_FRAME_SIZE  18U
#define TOUCAN_USB_TX_DATA_FRAME_CNT  (TOUCAN_USB_TX_DATA_PIPE_SIZE / TOUCAN_USB_TX_DATA_FRAME_SIZE)
#define TOUCAN_USB_RX_DATA_FRAME_CNT  (TOUCAN_USB_RX_DATA_PIPE_SIZE / TOUCAN_USB_RX_DATA_FRAME_SIZE)
// TODO: end!

#ifndef OPTION_DRIVER_RCV_QUEUE_SIZE
#define TOUCAN_RCV_QUEUE_SIZE  65536U
#else
#define TOUCAN_RCV_QUEUE_SIZE  OPTION_DRIVER_RCV_QUEUE_SIZE
#endif
#ifndef OPTION_DRIVER_TRM_QUEUE_SIZE
#define TOUCAN_TRM_QUEUE_SIZE  65536U
#else
#define TOUCAN_TRM_QUEUE_SIZE  OPTION_DRIVER_TRM_QUEUE_SIZE
#endif
#define TOUCAN_MAX_NAME_LENGTH  256
#define TOUCAN_MAX_STRING_LENGTH  80

#define TOUCAN_ERROR_SUCCESS  0
#define TOUCAN_ERROR_OFFSET  (-100)

/* ---  general CAN data types and defines  ---
 */
#if (OPTION_CANAPI_DRIVER != 0)
    #include "CANAPI_Types.h"
    /* typedefs from CAN API V3 */
    typedef can_message_t TouCAN_CanMessage_t;
    typedef can_timestamp_t TouCAN_Timestamp_t;
#else
    /* typedefs from <3rdParty.h> */
    // TODO: insert coin here
#endif

/* --- Defines and typdefs copied from CANAL interface DLL (CTouCANobj.h) ---
 *
 * CANAL interface DLL for RUSOKU technologies for TouCAN, TouCAN Marine, TouCAN Duo USB to CAN bus converter
 *
 * Copyright (C) 2000-2008 Ake Hedman, eurosource, <akhe@eurosource.se>
 * Copyright (C) 2020 Gediminas Simanskis (gediminas@rusoku.com)
 * Copyright (C) 2020 Alexander Sorokin (sorockin@yandex.ru)
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
/////////////////////////////////////////////////////
// USB request types & mask defines

#define        USB_HOST_TO_DEVICE                            0x00
#define        USB_DEVICE_TO_HOST                            0x80

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
#define        TouCAN_CAN_INTERFACE_INIT                    0x01 // OK
#define        TouCAN_CAN_INTERFACE_DEINIT                    0x02 // OK
#define        TouCAN_CAN_INTERFACE_START                    0x03 // OK
#define        TouCAN_CAN_INTERFACE_STOP                    0x04 // OK

#define        TouCAN_FILTER_STD_ACCEPT_ALL                0x05
#define        TouCAN_FILTER_STD_REJECT_ALL                0x06

#define        TouCAN_FILTER_EXT_ACCEPT_ALL                0x07
#define        TouCAN_FILTER_EXT_REJECT_ALL                0x08

#define        TouCAN_SET_FILTER_STD_LIST_MASK               0x09
#define        TouCAN_SET_FILTER_EXT_LIST_MASK                0x0A
#define        TouCAN_GET_FILTER_STD_LIST_MASK               0x0B
#define        TouCAN_GET_FILTER_EXT_LIST_MASK                0x0C

#define        TouCAN_GET_CAN_ERROR_STATUS                  0x0D  // OK   - CAN_Error_Status(&hcan1)
#define        TouCAN_CLEAR_CAN_ERROR_STATUS                0x0D  // NOK  - CAN_Error_Status(&hcan1) // nenusistato
#define        TouCAN_GET_STATISTICS                        0x0E  // OK (VSCP CAN state)
#define     TouCAN_CLEAR_STATISTICS                        0x0F  // OK (VSCP CAN state)
#define        TouCAN_GET_HARDWARE_VERSION                    0x10  // OK
#define        TouCAN_GET_FIRMWARE_VERSION                    0x11  // OK
#define        TouCAN_GET_BOOTLOADER_VERSION                0x12  // OK
#define        TouCAN_GET_SERIAL_NUMBER                    0x13  // OK
//#define        TouCAN_SET_SERIAL_NUMBER                    0x14
//#define        TouCAN_RESET_SERIAL_NUMBER                    0x15
#define        TouCAN_GET_VID_PID                            0x16  // OK
#define        TouCAN_GET_DEVICE_ID                        0x17  // OK
#define        TouCAN_GET_VENDOR                              0x18  // OK

#define        TouCAN_GET_LAST_ERROR_CODE                    0x20  // HAL return error code    8bit // OK
#define        TouCAN_CLEAR_LAST_ERROR_CODE                0x21  // HAL return error code  8bit // OK
#define        TouCAN_GET_CAN_INTERFACE_STATE                 0x22  // HAL_CAN_GetState(&hcan1)        8bit   // OK
#define        TouCAN_CLEAR_CAN_INTERFACE_STATE             0x23  // HAL_CAN_GetState(&hcan1); ----------------------------- NOK
#define        TouCAN_GET_CAN_INTERFACE_ERROR_CODE            0x24  // hcan->ErrorCode;    32bit    // OK  HAL_CAN_GetError(&hcan1);
#define        TouCAN_CLEAR_CAN_INTERFACE_ERROR_CODE        0x25  // hcan->ErrorCode;    32bit    // OK HAL_CAN_GetError(&hcan1);

#define     TouCAN_SET_CAN_INTERFACE_DELAY              0x26  // OK
#define     TouCAN_GET_CAN_INTERFACE_DELAY              0x27  // OK

///////////////////////////////////////////////////////
// TouCAN  return error codes (HAL)

#define     TouCAN_RETVAL_OK                            0x00
#define     TouCAN_RETVAL_ERROR                         0x01
#define     TouCAN_RETVAL_BUSY                            0x02
#define     TouCAN_RETVAL_TIMEOUT                       0x03

//////////////////////////////////////////////////////
//  TouCAN init string FLAGS (32 bit)

#define        TouCAN_ENABLE_SILENT_MODE                    0x00000001 //     1
#define        TouCAN_ENABLE_LOOPBACK_MODE                    0x00000002 //     2
#define        TouCAN_DISABLE_RETRANSMITION                0x00000004 //     4
#define        TouCAN_ENABLE_AUTOMATIC_WAKEUP_MODE            0x00000008 //     8
#define        TouCAN_ENABLE_AUTOMATIC_BUS_OFF                0x00000010 //    16
#define        TouCAN_ENABLE_TTM_MODE                        0x00000020 //    32
#define        TouCAN_ENABLE_RX_FIFO_LOCKED_MODE            0x00000040 //    64
#define        TouCAN_ENABLE_TX_FIFO_PRIORITY                 0x00000080 //  128

#define     TouCAN_ENABLE_STATUS_MESSAGES               0x00000100 //  256
#define     TouCAN_ENABLE_TIMESTAMP_DELAY               0x00000200 //  512

/////////////////////////////////////////////////////
// TouCAN HAL return error codes

typedef enum
{
    HAL_OK =      0x00U,
    HAL_ERROR =   0x01U,
    HAL_BUSY =      0x02U,
    HAL_TIMEOUT = 0x03U
} HAL_StatusTypeDef;


/////////////////////////////////////////////////////
// TouCAN CAN interface state

typedef enum
{
    HAL_CAN_STATE_RESET =            0x00U,  /*!< CAN not yet initialized or disabled */
    HAL_CAN_STATE_READY =            0x01U,  /*!< CAN initialized and ready for use   */
    HAL_CAN_STATE_LISTENING =        0x02U,  /*!< CAN receive process is ongoing      */
    HAL_CAN_STATE_SLEEP_PENDING =    0x03U,  /*!< CAN sleep request is pending        */
    HAL_CAN_STATE_SLEEP_ACTIVE =    0x04U,  /*!< CAN sleep mode is active            */
    HAL_CAN_STATE_ERROR =            0x05U   /*!< CAN error state                     */

} HAL_CAN_StateTypeDef;


////////////////////////////////////////////////////
//  TouCAN CAN interface ERROR codes

#define HAL_CAN_ERROR_NONE            (0x00000000U)  /*!< No error                                             */
#define HAL_CAN_ERROR_EWG             (0x00000001U)  /*!< Protocol Error Warning                               */
#define HAL_CAN_ERROR_EPV             (0x00000002U)  /*!< Error Passive                                        */
#define HAL_CAN_ERROR_BOF             (0x00000004U)  /*!< Bus-off error                                        */
#define HAL_CAN_ERROR_STF             (0x00000008U)  /*!< Stuff error                                          */
#define HAL_CAN_ERROR_FOR             (0x00000010U)  /*!< Form error                                           */
#define HAL_CAN_ERROR_ACK             (0x00000020U)  /*!< Acknowledgment error                                 */
#define HAL_CAN_ERROR_BR              (0x00000040U)  /*!< Bit recessive error                                  */
#define HAL_CAN_ERROR_BD              (0x00000080U)  /*!< Bit dominant error                                   */
#define HAL_CAN_ERROR_CRC             (0x00000100U)  /*!< CRC error                                            */
#define HAL_CAN_ERROR_RX_FOV0         (0x00000200U)  /*!< Rx FIFO0 overrun error                               */
#define HAL_CAN_ERROR_RX_FOV1         (0x00000400U)  /*!< Rx FIFO1 overrun error                               */
#define HAL_CAN_ERROR_TX_ALST0        (0x00000800U)  /*!< TxMailbox 0 transmit failure due to arbitration lost */
#define HAL_CAN_ERROR_TX_TERR0        (0x00001000U)  /*!< TxMailbox 1 transmit failure due to tranmit error    */
#define HAL_CAN_ERROR_TX_ALST1        (0x00002000U)  /*!< TxMailbox 0 transmit failure due to arbitration lost */
#define HAL_CAN_ERROR_TX_TERR1        (0x00004000U)  /*!< TxMailbox 1 transmit failure due to tranmit error    */
#define HAL_CAN_ERROR_TX_ALST2        (0x00008000U)  /*!< TxMailbox 0 transmit failure due to arbitration lost */
#define HAL_CAN_ERROR_TX_TERR2        (0x00010000U)  /*!< TxMailbox 1 transmit failure due to tranmit error    */
#define HAL_CAN_ERROR_TIMEOUT         (0x00020000U)  /*!< Timeout error                                        */
#define HAL_CAN_ERROR_NOT_INITIALIZED (0x00040000U)  /*!< Peripheral not initialized                           */
#define HAL_CAN_ERROR_NOT_READY       (0x00080000U)  /*!< Peripheral not ready                                 */
#define HAL_CAN_ERROR_NOT_STARTED     (0x00100000U)  /*!< Peripheral not started                               */
#define HAL_CAN_ERROR_PARAM           (0x00200000U)  /*!< Parameter error                                      */

/* --- Defines and typdefs copied from CANAL interface DLL (vscp_common/canal_a.h) ---
 *
 * CANAL interface DLL for RUSOKU technologies for TouCAN, TouCAN Marine, TouCAN Duo USB to CAN bus converter
 *
 * Copyright (C) 2018 Gediminas Simanskis (gediminas@rusoku.com)
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
/* FILTER req type */
typedef enum {
  FILTER_ACCEPT_ALL   = 0,
  FILTER_REJECT_ALL,
  FILTER_VALUE,
}Filter_Type_TypeDef;

#endif /* TOUCAN_USB_COMMON_H_INCLUDED */

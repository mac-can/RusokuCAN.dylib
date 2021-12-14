/*
 *  TouCAN - macOS User-Space Driver for Rusoku TouCAN USB Interfaces
 *
 *  Copyright (C) 2020-2021  Uwe Vogt, UV Software, Berlin (info@mac-can.com)
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
#ifndef TOUCAN_USB_H_INCLUDED
#define TOUCAN_USB_H_INCLUDED

#include "MacCAN.h"
#include "MacCAN_Common.h"
#include "MacCAN_MsgQueue.h"
#include "MacCAN_IOUsbKit.h"
#include "MacCAN_Devices.h"
#include "MacCAN_Debug.h"

#define RUSOKU_VENDOR_ID  (UInt16)0x16D0
#define RUSOKU_TOUCAN_USB_ID  (UInt16)0x0EAC

#define TOUCAN_USB_VENDOR_NAME  "Rusoku technologijos UAB, Lithuania"
#define TOUCAN_USB_VENDOR_URL   "www.rusoku.com"
#define TOUCAN_USB_DEVICE_NAME  "TouCAN USB"
#define TOUCAN_USB_DEVICE_TYPE  (SInt32)1

#define TOUCAN_USB_TX_DATA_PIPE_REF  1U
#define TOUCAN_USB_RX_DATA_PIPE_REF  2U
#define TOUCAN_USB_TX_DATA_PIPE_SIZE  64U
#define TOUCAN_USB_RX_DATA_PIPE_SIZE  64U
#define TOUCAN_USB_TX_DATA_FRAME_SIZE  18U
#define TOUCAN_USB_RX_DATA_FRAME_SIZE  18U
#define TOUCAN_USB_TX_DATA_FRAME_CNT  (TOUCAN_USB_TX_DATA_PIPE_SIZE / TOUCAN_USB_TX_DATA_FRAME_SIZE)
#define TOUCAN_USB_RX_DATA_FRAME_CNT  (TOUCAN_USB_RX_DATA_PIPE_SIZE / TOUCAN_USB_RX_DATA_FRAME_SIZE)

#define TOUCAN_USB_RCV_QUEUE_SIZE  65536
#define TOUCAN_USB_TRM_QUEUE_SIZE  256

#define TOUCAN_USB_MAX_FRAME_CNT  3
#define TOUCAN_USB_MAX_FRAME_DLC  CAN_MAX_DLC
#define TOUCAN_USB_MAX_FRAME_LEN  CAN_MAX_LEN

#define TOUCAN_MSG_STD_MAX_ID  CAN_MAX_STD_ID
#define TOUCAN_MSG_XTD_MAX_ID  CAN_MAX_XTD_ID

#define TOUCAN_USB_CLOCK_DOMAIN  50000000

#define TOUCAN_ERROR_SUCCESS  0
#define TOUCAN_ERROR_OFFSET  (-100)

typedef struct receive_param_t {  ///< - additional reception data:
    uint64_t m_u64StartTime;  ///< - time synchronization
    uint8_t m_u8StatusByte;  ///< - status byte from device
} TouCAN_MsgParam_t;

typedef struct receive_data_t {  ///< reception data:
    CANQUE_MsgQueue_t m_MsgQueue;  ///< - message queue
    TouCAN_MsgParam_t m_MsgParam;  ///< - additional data
} TouCAN_ReceiveData_t;

typedef CANUSB_AsyncPipe_t TouCAN_ReceivePipe_t;


// The following definitions were copied from CANAL (CAN Abstraction Layer)
// <http://can.sourceforge.net>
//
// Copyright (C) 2000-2008 Ake Hedman, eurosource, <akhe@eurosource.se>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
#if (OPTION_TOUCAN_CANAL != 0)

/// ID flags
#define CANAL_IDFLAG_STANDARD                0x00000000    // Standard message id (11-bit)
#define CANAL_IDFLAG_EXTENDED                0x00000001    // Extended message id (29-bit)
#define CANAL_IDFLAG_RTR                        0x00000002    // RTR-Frame
#define CANAL_IDFLAG_STATUS                    0x00000004    // This package is a status indication (id holds error code)
#define CANAL_IDFLAG_SEND                        0x80000000  // Reserved for use by application software to indicate send

/// Communicaton speeds
#define CANAL_BAUD_USER                        0                        // User specified (In CANAL i/f DLL).
#define CANAL_BAUD_1000                        1                        //   1 Mbit
#define CANAL_BAUD_800                        2                        // 800 Kbit
#define CANAL_BAUD_500                        3                        // 500 Kbit
#define CANAL_BAUD_250                        4                        // 250 Kbit
#define CANAL_BAUD_125                        5                        // 125 Kbit
#define CANAL_BAUD_100                        6                        // 100 Kbit
#define CANAL_BAUD_50                          7                        //  50 Kbit
#define CANAL_BAUD_20                          8                        //  20 Kbit
#define CANAL_BAUD_10                          9                        //  10 Kbit

/// Status message codes ( in received message )
#define CANAL_STATUSMSG_OK                0x00    // Normal condition.
#define CANAL_STATUSMSG_OVERRUN        0x01    // Overrun occured when sending data to CAN bus.
#define CANAL_STATUSMSG_BUSLIGHT    0x02    // Error counter has reached 96.
#define CANAL_STATUSMSG_BUSHEAVY    0x03    // Error counter has reached 128.
#define CANAL_STATUSMSG_BUSOFF        0x04    // Device is in BUSOFF. CANAL_STATUS_OK is
                                                                                // sent when returning to operational mode.
#define CANAL_STATUSMSG_STUFF            0x20    // Stuff Error.
#define CANAL_STATUSMSG_FORM            0x21    // Form Error.
#define CANAL_STATUSMSG_ACK                0x23    // Ack Error.
#define CANAL_STATUSMSG_BIT1            0x24    // Bit1 Error.
#define CANAL_STATUSMSG_BIT0            0x25    // Bit0 Error.
#define CANAL_STATUSMSG_CRC                0x27    // CRC Error.

/// Status codes /returned by status request)
#define CANAL_STATUS_NONE                    0x00000000
#define CANAL_STATUS_ACTIVE                    0x10000000
#define CANAL_STATUS_PASSIVE                0x40000000
#define CANAL_STATUS_BUS_OFF                0x80000000
#define CANAL_STATUS_BUS_WARN                0x20000000
#define CANAL_STATUS_PHY_FAULT                0x08000000
#define CANAL_STATUS_PHY_H                    0x04000000
#define CANAL_STATUS_PHY_L                    0x02000000
#define CANAL_STATUS_SLEEPING                0x01000000
#define CANAL_STATUS_STOPPED                0x00800000
#define CANAL_STATUS_RECIVE_BUFFER_FULL        0x00400000    // Drivers buffer
#define CANAL_STATUS_TRANSMIT_BUFFER_FULL    0x00200000    // Drivers buffer

/// Error Codes
#define CANAL_ERROR_SUCCESS                        0        // All is OK
#define CANAL_ERROR_BAUDRATE                    1        // Baudrate error
#define CANAL_ERROR_BUS_OFF                        2        // Bus off error
#define CANAL_ERROR_BUS_PASSIVE                    3        // Bus Passive error
#define CANAL_ERROR_BUS_WARNING                    4        // Bus warning error
#define CANAL_ERROR_CAN_ID                        5        // Invalid CAN ID
#define CANAL_ERROR_CAN_MESSAGE                    6        // Invalid CAN message
#define CANAL_ERROR_CHANNEL                        7        // Invalid channel
#define CANAL_ERROR_FIFO_EMPTY                    8        // FIFO is empty
#define CANAL_ERROR_FIFO_FULL                    9        // FIFI is full
#define CANAL_ERROR_FIFO_SIZE                    10        // FIFO size error
#define CANAL_ERROR_FIFO_WAIT                    11
#define CANAL_ERROR_GENERIC                        12        // Generic error
#define CANAL_ERROR_HARDWARE                    13        // Hardware error
#define CANAL_ERROR_INIT_FAIL                    14        // Initialization failed
#define CANAL_ERROR_INIT_MISSING                15
#define CANAL_ERROR_INIT_READY                    16
#define CANAL_ERROR_NOT_SUPPORTED                17        // Not supported
#define CANAL_ERROR_OVERRUN                        18        // Overrun
#define CANAL_ERROR_RCV_EMPTY                    19        // Receive buffer empty
#define CANAL_ERROR_REGISTER                    20        // Register value error
#define CANAL_ERROR_TRM_FULL                    21
#define CANAL_ERROR_ERRFRM_STUFF                22        // Errorframe: stuff error detected
#define CANAL_ERROR_ERRFRM_FORM                    23        // Errorframe: form error detected
#define CANAL_ERROR_ERRFRM_ACK                    24        // Errorframe: acknowledge error
#define CANAL_ERROR_ERRFRM_BIT1                    25        // Errorframe: bit 1 error
#define CANAL_ERROR_ERRFRM_BIT0                    26        // Errorframe: bit 0 error
#define CANAL_ERROR_ERRFRM_CRC                    27        // Errorframe: CRC error
#define CANAL_ERROR_LIBRARY                        28        // Unable to load library
#define CANAL_ERROR_PROCADDRESS                    29        // Unable get library proc address
#define CANAL_ERROR_ONLY_ONE_INSTANCE            30        // Only one instance allowed
#define CANAL_ERROR_SUB_DRIVER                    31        // Problem with sub driver call
#define CANAL_ERROR_TIMEOUT                        32        // Blocking call timeout
#define CANAL_ERROR_NOT_OPEN                    33         // The device is not open.
#define CANAL_ERROR_PARAMETER                    34         // A parameter is invalid.
#define CANAL_ERROR_MEMORY                        35         // Memory exhausted.
#define CANAL_ERROR_INTERNAL                    36         // Some kind of internal program error
#define CANAL_ERROR_COMMUNICATION                37         // Some kind of communication error
#define CANAL_ERROR_USER                        38      // Login error

/*!
    CanalStatistics

    This is the general statistics structure
*/

typedef struct structCanalStatistics {
    unsigned long cntReceiveFrames;                // # of receive frames
    unsigned long cntTransmitFrames;            // # of transmitted frames
    unsigned long cntReceiveData;                  // # of received data bytes
    unsigned long cntTransmitData;                // # of transmitted data bytes
    unsigned long cntOverruns;                    // # of overruns
    unsigned long cntBusWarnings;                  // # of bys warnings
    unsigned long cntBusOff;                    // # of bus off's
} canalStatistics;

typedef  canalStatistics *PCANALSTATISTICS;

/*!
    CanalStatus

    This is the general channel state structure
*/

typedef struct structCanalStatus {
    unsigned long channel_status;          // Current state for channel
    unsigned long lasterrorcode;         // Last error code
    unsigned long lasterrorsubcode;        // Last error subcode
    char lasterrorstr[80];                // Last error string
} canalStatus;


typedef  canalStatus * PCANALSTATUS;

#endif

/* The following source code was copied from CANAL-DLL for Windows
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
///////////////////////////////////////////////////////
// TouCAN  return error codes (HAL)

#define        TouCAN_RETVAL_OK                            0x00
#define        TouCAN_RETVAL_ERROR                         0x01
#define        TouCAN_RETVAL_BUSY                          0x02
#define        TouCAN_RETVAL_TIMEOUT                       0x03

//////////////////////////////////////////////////////
// TouCAN init string FLAGS (32 bit)

#define        TouCAN_ENABLE_SILENT_MODE                   0x00000001 //    1
#define        TouCAN_ENABLE_LOOPBACK_MODE                 0x00000002 //    2
#define        TouCAN_DISABLE_RETRANSMITION                0x00000004 //    4
#define        TouCAN_ENABLE_AUTOMATIC_WAKEUP_MODE         0x00000008 //    8
#define        TouCAN_ENABLE_AUTOMATIC_BUS_OFF             0x00000010 //   16
#define        TouCAN_ENABLE_TTM_MODE                      0x00000020 //   32
#define        TouCAN_ENABLE_RX_FIFO_LOCKED_MODE           0x00000040 //   64
#define        TouCAN_ENABLE_TX_FIFO_PRIORITY              0x00000080 //  128

#define        TouCAN_ENABLE_STATUS_MESSAGES               0x00000100 //  256
#define        TouCAN_ENABLE_TIMESTAMP_DELAY               0x00000200 //  512

////////////////////////////////////////////////////
//  TouCAN CAN interface ERROR codes

#define HAL_CAN_ERROR_NONE            (0x00000000U)  /* No error                                             */
#define HAL_CAN_ERROR_EWG             (0x00000001U)  /* Protocol Error Warning                               */
#define HAL_CAN_ERROR_EPV             (0x00000002U)  /* Error Passive                                        */
#define HAL_CAN_ERROR_BOF             (0x00000004U)  /* Bus-off error                                        */
#define HAL_CAN_ERROR_STF             (0x00000008U)  /* Stuff error                                          */
#define HAL_CAN_ERROR_FOR             (0x00000010U)  /* Form error                                           */
#define HAL_CAN_ERROR_ACK             (0x00000020U)  /* Acknowledgment error                                 */
#define HAL_CAN_ERROR_BR              (0x00000040U)  /* Bit recessive error                                  */
#define HAL_CAN_ERROR_BD              (0x00000080U)  /* Bit dominant error                                   */
#define HAL_CAN_ERROR_CRC             (0x00000100U)  /* CRC error                                            */
#define HAL_CAN_ERROR_RX_FOV0         (0x00000200U)  /* Rx FIFO0 overrun error                               */
#define HAL_CAN_ERROR_RX_FOV1         (0x00000400U)  /* Rx FIFO1 overrun error                               */
#define HAL_CAN_ERROR_TX_ALST0        (0x00000800U)  /* TxMailbox 0 transmit failure due to arbitration lost */
#define HAL_CAN_ERROR_TX_TERR0        (0x00001000U)  /* TxMailbox 1 transmit failure due to tranmit error    */
#define HAL_CAN_ERROR_TX_ALST1        (0x00002000U)  /* TxMailbox 0 transmit failure due to arbitration lost */
#define HAL_CAN_ERROR_TX_TERR1        (0x00004000U)  /* TxMailbox 1 transmit failure due to tranmit error    */
#define HAL_CAN_ERROR_TX_ALST2        (0x00008000U)  /* TxMailbox 0 transmit failure due to arbitration lost */
#define HAL_CAN_ERROR_TX_TERR2        (0x00010000U)  /* TxMailbox 1 transmit failure due to tranmit error    */
#define HAL_CAN_ERROR_TIMEOUT         (0x00020000U)  /* Timeout error                                        */
#define HAL_CAN_ERROR_NOT_INITIALIZED (0x00040000U)  /* Peripheral not initialized                           */
#define HAL_CAN_ERROR_NOT_READY       (0x00080000U)  /* Peripheral not ready                                 */
#define HAL_CAN_ERROR_NOT_STARTED     (0x00100000U)  /* Peripheral not started                               */
#define HAL_CAN_ERROR_PARAM           (0x00200000U)  /* Parameter error                                      */

/* FILTER req type */
typedef enum {
  FILTER_ACCEPT_ALL   = 0,
  FILTER_REJECT_ALL,
  FILTER_VALUE,
}Filter_Type_TypeDef;

#ifdef __cplusplus
extern "C" {
#endif

extern int TouCAN_init(CANUSB_Handle_t hDevice, UInt16 u16Brp, UInt8 u8Tseg1, UInt8 u8Tseg2, UInt8 u8Sjw, UInt32 u32Flags);
extern int TouCAN_deinit(CANUSB_Handle_t hDevice);
extern int TouCAN_start(CANUSB_Handle_t hDevice);
extern int TouCAN_stop(CANUSB_Handle_t hDevice);

extern int TouCAN_get_last_error_code(CANUSB_Handle_t hDevice, UInt8 *res);             // HAL error code
extern int TouCAN_get_interface_error_code(CANUSB_Handle_t hDevice, UInt32 *ErrorCode);    // hcan->ErrorCode;
extern int TouCAN_clear_interface_error_code(CANUSB_Handle_t hDevice);                     // hcan->ErrorCode;
extern int TouCAN_get_interface_state(CANUSB_Handle_t hDevice, UInt8 *state);              // hcan->State;
#if (OPTION_TOUCAN_CANAL != 0)
extern int TouCAN_get_statistics(CANUSB_Handle_t hDevice, PCANALSTATISTICS statistics);   // VSCP get statistics
extern int TouCAN_clear_statistics(CANUSB_Handle_t hDevice);                               // VSCP clear statistics
extern int TouCAN_get_canal_status(CANUSB_Handle_t hDevice, canalStatus *status);
#endif
extern int TouCAN_get_hardware_version(CANUSB_Handle_t hDevice, UInt32 *ver);
extern int TouCAN_get_firmware_version(CANUSB_Handle_t hDevice, UInt32 *ver);
extern int TouCAN_get_bootloader_version(CANUSB_Handle_t hDevice, UInt32 *ver);
extern int TouCAN_get_serial_number(CANUSB_Handle_t hDevice, UInt32 *ver);
extern int TouCAN_get_vid_pid(CANUSB_Handle_t hDevice, UInt32 *ver);
extern int TouCAN_get_device_id(CANUSB_Handle_t hDevice, UInt32 *ver);
extern int TouCAN_get_vendor(CANUSB_Handle_t hDevice, unsigned int size, char *str);
extern int TouCAN_get_interface_transmit_delay(CANUSB_Handle_t hDevice, UInt8 channel,UInt32 *delay);
extern int TouCAN_set_interface_transmit_delay(CANUSB_Handle_t hDevice, UInt8 channel,UInt32 *delay);

extern int TouCAN_set_filter_std_list_mask(CANUSB_Handle_t hDevice, Filter_Type_TypeDef type, UInt32 list, UInt32 mask);
extern int TouCAN_set_filter_ext_list_mask(CANUSB_Handle_t hDevice, Filter_Type_TypeDef type, UInt32 list, UInt32 mask);

/*
 *  That's my bullshit again;)
 */
extern int TouCAN_StartReception(TouCAN_ReceivePipe_t pipe, const TouCAN_ReceiveData_t *context);
extern int TouCAN_AbortReception(TouCAN_ReceivePipe_t pipe);

extern int TouCAN_EncodeMessage(UInt8 *buffer, const MacCAN_Message_t *message);
extern int TouCAN_DecodeMessage(MacCAN_Message_t *message, const UInt8 *buffer, TouCAN_MsgParam_t *param);

extern int TouCAN_InitializeInterface(CANUSB_Handle_t hDevice);
extern int TouCAN_TeardownInterface(CANUSB_Handle_t hDevice);

extern int TouCAN_SetBitrateAndMode(CANUSB_Handle_t hDevice, const MacCAN_Bitrate_t *bitrate,
                                                             const MacCAN_OpMode_t *opMode);

extern int TouCAN_StartInterface(CANUSB_Handle_t hDevice);
extern int TouCAN_StopInterface(CANUSB_Handle_t hDevice);

#ifdef __cplusplus
}
#endif
#endif /* TOUCAN_USB_H_INCLUDED */

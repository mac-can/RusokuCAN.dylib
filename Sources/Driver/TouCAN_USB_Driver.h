/*  SPDX-License-Identifier: GPL-3.0-or-later */
/*
 *  TouCAN - macOS User-Space Driver for Rusoku TouCAN USB Interfaces
 *
 *  Copyright (C) 2020-2023  Uwe Vogt, UV Software, Berlin (info@mac-can.com)
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
#ifndef TOUCAN_USB_DRIVER_H_INCLUDED
#define TOUCAN_USB_DRIVER_H_INCLUDED

#include "TouCAN_USB_Common.h"
#include "TouCAN_USB_Device.h"

#define TOUCAN_USB_VENDOR_ID   TOUCAN_VENDOR_ID
#define TOUCAN_USB_PRODUCT_ID  TOUCAN_USB_ID

#define TOUCAN_USB_DEVICE_TYPE  (SInt32)1
#define TOUCAN_USB_DEVICE_NAME  "TouCAN USB"

#define TOUCAN_USB_NUM_CHANNELS   1U
#define TOUCAN_USB_NUM_ENDPOINTS  2U

#define TOUCAN_USB_MAX_FRAME_CNT  3
#define TOUCAN_USB_MAX_FRAME_DLC  CAN_MAX_DLC
#define TOUCAN_USB_MAX_FRAME_LEN  CAN_MAX_LEN

#define TOUCAN_USB_CPU_FREQUENCY  50U
#define TOUCAN_USB_BTR_BRP_250K   10U
#define TOUCAN_USB_BTR_TSEG1_250K 14U
#define TOUCAN_USB_BTR_TSEG2_250K 5U
#define TOUCAN_USB_BTR_SJW_250K   4U

#define TOUCAN_USB_MODE_FDOE  0 /* CAN FD operation enable/disable */
#define TOUCAN_USB_MODE_BRSE  0 /* bit-rate switch enable/disable */
#define TOUCAN_USB_MODE_NISO  0 /* Non-ISO CAN FD enable/disable */
#define TOUCAN_USB_MODE_SHRD  0 /* shared access enable/disable */
#define TOUCAN_USB_MODE_NXTD  1 /* extended format disable/enable */
#define TOUCAN_USB_MODE_NRTR  1 /* remote frames disable/enable */
#define TOUCAN_USB_MODE_ERR   1 /* error frames enable/disable */
#define TOUCAN_USB_MODE_MON   1 /* monitor mode enable/disable */

#ifdef __cplusplus
extern "C" {
#endif

extern bool TouCAN_USB_ConfigureChannel(TouCAN_Device_t *device);
extern void TouCAN_USB_GetOperationCapability(TouCAN_OpMode_t *capability);

extern CANUSB_Return_t TouCAN_USB_InitializeChannel(TouCAN_Device_t *device, TouCAN_OpMode_t mode);
extern CANUSB_Return_t TouCAN_USB_TeardownChannel(TouCAN_Device_t *device);

extern CANUSB_Return_t TouCAN_USB_SetBitrate(TouCAN_Device_t *device, const TouCAN_Bitrate_t *bitrate);
extern CANUSB_Return_t TouCAN_USB_StartCan(TouCAN_Device_t *device);
extern CANUSB_Return_t TouCAN_USB_StopCan(TouCAN_Device_t *device);

extern CANUSB_Return_t TouCAN_USB_WriteMessage(TouCAN_Device_t *device, const TouCAN_CanMessage_t *message, uint16_t timeout);
extern CANUSB_Return_t TouCAN_USB_ReadMessage(TouCAN_Device_t *device, TouCAN_CanMessage_t *message, uint16_t timeout);
extern CANUSB_Return_t TouCAN_USB_GetBusStatus(TouCAN_Device_t *device, TouCAN_Status_t *status);

extern bool TouCAN_USB_Index2Bitrate(int32_t index, TouCAN_Bitrate_t *bitrate);

#ifdef __cplusplus
}
#endif

#endif /* TOUCAN_USB_DRIVER_H_INCLUDED */

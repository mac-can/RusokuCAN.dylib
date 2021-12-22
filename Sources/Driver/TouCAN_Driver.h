/*  SPDX-License-Identifier: GPL-3.0-or-later */
/*
 *  TouCAN - macOS User-Space Driver for Rusoku TouCAN USB Interfaces
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
#ifndef TOUCAN_DRIVER_H_INCLUDED
#define TOUCAN_DRIVER_H_INCLUDED

#include "TouCAN_USB_Common.h"
#include "TouCAN_USB_Device.h"

typedef int32_t TouCAN_Channel_t;

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t TouCAN_DriverVersion(void);
extern CANUSB_Return_t TouCAN_InitializeDriver(void);
extern CANUSB_Return_t TouCAN_TeardownDriver(void);

extern CANUSB_Return_t TouCAN_ProbeChannel(TouCAN_Channel_t channel, TouCAN_OpMode_t mode, int *state);
extern CANUSB_Return_t TouCAN_InitializeChannel(TouCAN_Channel_t channel, TouCAN_OpMode_t opMode, TouCAN_Device_t *device);
extern CANUSB_Return_t TouCAN_TeardownChannel(TouCAN_Device_t *device);
extern CANUSB_Return_t TouCAN_SignalChannel(TouCAN_Device_t *device);

extern CANUSB_Return_t TouCAN_SetBitrate(TouCAN_Device_t *device, const TouCAN_Bitrate_t *bitrate);
extern CANUSB_Return_t TouCAN_StartCan(TouCAN_Device_t *device);
extern CANUSB_Return_t TouCAN_StopCan(TouCAN_Device_t *device);

extern CANUSB_Return_t TouCAN_WriteMessage(TouCAN_Device_t *device, const TouCAN_CanMessage_t *message, uint16_t timeout);
extern CANUSB_Return_t TouCAN_ReadMessage(TouCAN_Device_t *device, TouCAN_CanMessage_t *message, uint16_t timeout);
extern CANUSB_Return_t TouCAN_GetBusStatus(TouCAN_Device_t *device, TouCAN_Status_t *status);

extern bool TouCAN_Index2Bitrate(TouCAN_Device_t *device, int32_t index, TouCAN_Bitrate_t *bitrate);

#ifdef __cplusplus
}
#endif

#endif /* TOUCAN_DRIVER_H_INCLUDED */

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

#include "TouCAN_Common.h"

#include "MacCAN.h"
#include "MacCAN_Common.h"
#include "MacCAN_MsgQueue.h"
#include "MacCAN_IOUsbKit.h"
#include "MacCAN_Devices.h"
#include "MacCAN_Debug.h"

typedef struct receive_param_t {  ///< - additional reception data:
    uint64_t m_u64StartTime;  ///< - time synchronization
    uint8_t m_u8StatusByte;  ///< - status byte from device
} TouCAN_MsgParam_t;

typedef struct receive_data_t {  ///< reception data:
    CANQUE_MsgQueue_t m_MsgQueue;  ///< - message queue
    TouCAN_MsgParam_t m_MsgParam;  ///< - additional data
} TouCAN_ReceiveData_t;

typedef CANUSB_AsyncPipe_t TouCAN_ReceivePipe_t;


#ifdef __cplusplus
extern "C" {
#endif

extern int TouCAN_init(CANUSB_Handle_t handle, UInt16 brp, UInt8 tseg1, UInt8 tseg2, UInt8 sjw, UInt32 flags);
extern int TouCAN_deinit(CANUSB_Handle_t handle);
extern int TouCAN_start(CANUSB_Handle_t handle);
extern int TouCAN_stop(CANUSB_Handle_t handle);

extern int TouCAN_get_last_error_code(CANUSB_Handle_t handle, UInt8 *res);
extern int TouCAN_get_interface_error_code(CANUSB_Handle_t handle, UInt32 *ErrorCode);
extern int TouCAN_clear_interface_error_code(CANUSB_Handle_t handle);
extern int TouCAN_get_interface_state(CANUSB_Handle_t handle, UInt8 *state);
#if (OPTION_TOUCAN_CANAL != 0)
extern int TouCAN_get_statistics(CANUSB_Handle_t handle, PCANALSTATISTICS statistics);
extern int TouCAN_clear_statistics(CANUSB_Handle_t handle);
extern int TouCAN_get_canal_status(CANUSB_Handle_t handle, canalStatus *status);
#endif
extern int TouCAN_get_hardware_version(CANUSB_Handle_t handle, UInt32 *ver);
extern int TouCAN_get_firmware_version(CANUSB_Handle_t handle, UInt32 *ver);
extern int TouCAN_get_bootloader_version(CANUSB_Handle_t handle, UInt32 *ver);
extern int TouCAN_get_serial_number(CANUSB_Handle_t handle, UInt32 *ver);
extern int TouCAN_get_vid_pid(CANUSB_Handle_t handle, UInt32 *ver);
extern int TouCAN_get_device_id(CANUSB_Handle_t handle, UInt32 *ver);
extern int TouCAN_get_vendor(CANUSB_Handle_t handle, unsigned int size, char *str);
extern int TouCAN_get_interface_transmit_delay(CANUSB_Handle_t handle, UInt8 channel,UInt32 *delay);
extern int TouCAN_set_interface_transmit_delay(CANUSB_Handle_t handle, UInt8 channel,UInt32 *delay);

extern int TouCAN_set_filter_std_list_mask(CANUSB_Handle_t handle, Filter_Type_TypeDef type, UInt32 list, UInt32 mask);
extern int TouCAN_set_filter_ext_list_mask(CANUSB_Handle_t handle, Filter_Type_TypeDef type, UInt32 list, UInt32 mask);

/*
 *  That's my bullshit again;)
 */
extern int TouCAN_StartReception(TouCAN_ReceivePipe_t pipe, const TouCAN_ReceiveData_t *context);
extern int TouCAN_AbortReception(TouCAN_ReceivePipe_t pipe);

extern int TouCAN_EncodeMessage(UInt8 *buffer, const MacCAN_Message_t *message);
extern int TouCAN_DecodeMessage(MacCAN_Message_t *message, const UInt8 *buffer, TouCAN_MsgParam_t *param);

extern int TouCAN_InitializeInterface(CANUSB_Handle_t handle);
extern int TouCAN_TeardownInterface(CANUSB_Handle_t handle);

extern int TouCAN_SetBitrateAndMode(CANUSB_Handle_t handle, const MacCAN_Bitrate_t *bitrate,
                                                            const MacCAN_OpMode_t *opMode);

#ifdef __cplusplus
}
#endif
#endif /* TOUCAN_USB_H_INCLUDED */

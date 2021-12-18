/*  SPDX-License-Identifier: GPL-3.0-or-later */
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

#include "TouCAN_USB_Common.h"

typedef int TouCAN_Handle_t;

#ifdef __cplusplus
extern "C" {
#endif

extern int TouCAN_init(TouCAN_Handle_t handle, uint16_t brp, uint8_t tseg1, uint8_t tseg2, uint8_t sjw, uint32_t flags);
extern int TouCAN_deinit(TouCAN_Handle_t handle);
extern int TouCAN_start(TouCAN_Handle_t handle);
extern int TouCAN_stop(TouCAN_Handle_t handle);

extern int TouCAN_get_last_error_code(TouCAN_Handle_t handle, uint8_t *res);
extern int TouCAN_get_interface_error_code(TouCAN_Handle_t handle, uint32_t *ErrorCode);
extern int TouCAN_clear_interface_error_code(TouCAN_Handle_t handle);
extern int TouCAN_get_interface_state(TouCAN_Handle_t handle, uint8_t *state);
#if (OPTION_TOUCAN_CANAL != 0)
extern int TouCAN_get_statistics(TouCAN_Handle_t handle, PCANALSTATISTICS statistics);
extern int TouCAN_clear_statistics(TouCAN_Handle_t handle);
extern int TouCAN_get_canal_status(TouCAN_Handle_t handle, canalStatus *status);
#endif
extern int TouCAN_get_hardware_version(TouCAN_Handle_t handle, uint32_t *ver);
extern int TouCAN_get_firmware_version(TouCAN_Handle_t handle, uint32_t *ver);
extern int TouCAN_get_bootloader_version(TouCAN_Handle_t handle, uint32_t *ver);
extern int TouCAN_get_serial_number(TouCAN_Handle_t handle, uint32_t *ver);
extern int TouCAN_get_vid_pid(TouCAN_Handle_t handle, uint32_t *ver);
extern int TouCAN_get_device_id(TouCAN_Handle_t handle, uint32_t *ver);
extern int TouCAN_get_vendor(TouCAN_Handle_t handle, unsigned int size, char *str);
extern int TouCAN_get_interface_transmit_delay(TouCAN_Handle_t handle, uint8_t channel,uint32_t *delay);
extern int TouCAN_set_interface_transmit_delay(TouCAN_Handle_t handle, uint8_t channel,uint32_t *delay);

extern int TouCAN_set_filter_std_list_mask(TouCAN_Handle_t handle, Filter_Type_TypeDef type, uint32_t list, uint32_t mask);
extern int TouCAN_set_filter_ext_list_mask(TouCAN_Handle_t handle, Filter_Type_TypeDef type, uint32_t list, uint32_t mask);

#ifdef __cplusplus
}
#endif
#endif /* TOUCAN_USB_H_INCLUDED */

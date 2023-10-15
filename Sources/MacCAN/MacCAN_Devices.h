/*  SPDX-License-Identifier: BSD-2-Clause OR GPL-3.0-or-later */
/*
 *  MacCAN - macOS User-Space Driver for USB-to-CAN Interfaces
 *
 *  Copyright (c) 2012-2022 Uwe Vogt, UV Software, Berlin (info@mac-can.com)
 *  All rights reserved.
 *
 *  This file is part of MacCAN-Core.
 *
 *  MacCAN-Core is dual-licensed under the BSD 2-Clause "Simplified" License and
 *  under the GNU General Public License v3.0 (or any later version).
 *  You can choose between one of them if you use this file.
 *
 *  BSD 2-Clause "Simplified" License:
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  MacCAN-Core IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF MacCAN-Core, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  GNU General Public License v3.0 or later:
 *  MacCAN-Core is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  MacCAN-Core is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with MacCAN-Core.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MACCAN_DEVICES_H_INCLUDED
#define MACCAN_DEVICES_H_INCLUDED

#include "MacCAN_Common.h"

#define CANDEV_LAST_ENTRY_IN_DEVICE_LIST  {0xFFFFU, 0xFFFFU, 0U, NULL, NULL}

typedef int CANDEV_Index_t;

typedef void *CANDEV_Descriptor_t;
typedef void (*CANDEV_Callback_t)(CANDEV_Index_t index, CANDEV_Descriptor_t *descriptor);

typedef struct can_device_tag {
    UInt16 vendorId;
    UInt16 productId;
    UInt8 numChannels;
    CANDEV_Callback_t cbkAdded;
    CANDEV_Callback_t cbkRemoved;
} CANDEV_Device_t, MacCAN_Device_t;

#ifdef __cplusplus
extern "C" {
#endif

extern const CANDEV_Device_t *CANDEV_GetFirstDevice(void);

extern const CANDEV_Device_t *CANDEV_GetNextDevice(void);

extern const CANDEV_Device_t *CANDEV_GetDeviceById(UInt16 vendorId, UInt16 productId);

extern UInt16 CANDEV_GetVendorId(const CANDEV_Device_t *device);

extern UInt16 CANDEV_GetProductId(const CANDEV_Device_t *device);

extern UInt8 CANDEV_GetNumChannels(const CANDEV_Device_t *device);

extern void CANDEV_DeviceAdded(const CANDEV_Device_t *device, CANDEV_Index_t index, CANDEV_Descriptor_t *descriptor);

extern void CANDEV_DeviceRemoved(const CANDEV_Device_t *device, CANDEV_Index_t index, CANDEV_Descriptor_t *descriptor);

#ifdef __cplusplus
}
#endif
#endif /* MACCAN_DEVICES_H_INCLUDED */

/* * $Id: MacCAN_Devices.h 1550 2022-10-06 16:34:34Z makemake $ *** (c) UV Software, Berlin ***
 */

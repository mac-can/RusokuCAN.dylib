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
#ifndef MACCAN_DEVICES_H_INCLUDED
#define MACCAN_DEVICES_H_INCLUDED

#include "MacCAN.h"

#include <MacTypes.h>

#ifndef MACCAN_H_INCLUDED
/* Hmm, that's pretty fragile! */
typedef struct can_device_t_ {
    UInt16 vendorId;
    UInt16 productId;
} CANDEV_Device_t;
#else
typedef MacCAN_Device_t CANDEV_Device_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern const CANDEV_Device_t *CANDEV_GetFirstDevice(void);

extern const CANDEV_Device_t *CANDEV_GetNextDevice(void);

extern const CANDEV_Device_t *CANDEV_GetDeviceById(UInt16 vendorId, UInt16 productId);

#ifdef __cplusplus
}
#endif
#endif /* MACCAN_DEVICES_H_INCLUDED */

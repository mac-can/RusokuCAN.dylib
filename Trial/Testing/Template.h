//
//  TouCAN - macOS User-Space Driver for Rusoku TouCAN USB Interfaces
//
//  Copyright (C) 2020-2021  Uwe Vogt, UV Software, Berlin (info@mac-can.com)
//
//  This file is part of MacCAN-TouCAN.
//
//  MacCAN-TouCAN is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  MacCAN-TouCAN is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with MacCAN-TouCAN.  If not, see <https://www.gnu.org/licenses/>.
//
//  IMPORTANT NOTE
//
//  CAN API V3 and CAN API V3 Testing is dual-licensed under the terms of the
//  BSD 2-Clause "Simplified" License and under the terms of the GNU General
//  Public License v3.0 (or any later version). The terms of the GNU General
//  Public License v3.0 (or any later version) apply to this work, see above.
//
#ifndef TEMPLATE_H_INCLUDED
#define TEMPLATE_H_INCLUDED

#include "TouCAN.h"

#define CDriverCAN  CTouCAN

#define CAN_DEVICE1  TOUCAN_USB_CHANNEL0
#define CAN_DEVICE2  TOUCAN_USB_CHANNEL1

#define BITRATE_1M(x)       do{ x.btr.frequency=80000000; x.btr.nominal.brp=2; x.btr.nominal.tseg1= 31; x.btr.nominal.tseg2= 8; x.btr.nominal.sjw= 8; } while(0)
#define BITRATE_500K(x)     do{ x.btr.frequency=80000000; x.btr.nominal.brp=2; x.btr.nominal.tseg1= 63; x.btr.nominal.tseg2=16; x.btr.nominal.sjw= 8; } while(0)
#define BITRATE_250K(x)     do{ x.btr.frequency=80000000; x.btr.nominal.brp=2; x.btr.nominal.tseg1=127; x.btr.nominal.tseg2=32; x.btr.nominal.sjw= 8; } while(0)
#define BITRATE_125K(x)     do{ x.btr.frequency=80000000; x.btr.nominal.brp=4; x.btr.nominal.tseg1=127; x.btr.nominal.tseg2=32; x.btr.nominal.sjw= 8; } while(0)

#define BITRATE_1M8M(x)     do{ x.btr.frequency=80000000; x.btr.nominal.brp=2; x.btr.nominal.tseg1= 31; x.btr.nominal.tseg2= 8; x.btr.nominal.sjw= 8; x.btr.data.brp=2; x.btr.data.tseg1= 3; x.btr.data.tseg2=1; x.btr.data.sjw=1; } while(0)
#define BITRATE_500K4M(x)   do{ x.btr.frequency=80000000; x.btr.nominal.brp=2; x.btr.nominal.tseg1= 63; x.btr.nominal.tseg2=16; x.btr.nominal.sjw= 8; x.btr.data.brp=2; x.btr.data.tseg1= 7; x.btr.data.tseg2=2; x.btr.data.sjw=2; } while(0)
#define BITRATE_250K2M(x)   do{ x.btr.frequency=80000000; x.btr.nominal.brp=2; x.btr.nominal.tseg1=127; x.btr.nominal.tseg2=32; x.btr.nominal.sjw= 8; x.btr.data.brp=2; x.btr.data.tseg1=15; x.btr.data.tseg2=4; x.btr.data.sjw=2; } while(0)
#define BITRATE_125K1M(x)   do{ x.btr.frequency=80000000; x.btr.nominal.brp=4; x.btr.nominal.tseg1=127; x.btr.nominal.tseg2=32; x.btr.nominal.sjw= 8; x.btr.data.brp=2; x.btr.data.tseg1=31; x.btr.data.tseg2=8; x.btr.data.sjw=2; } while(0)

#define OPTION_CIABTR_INDEX0_SUPPORTED  1
#define OPTION_CIABTR_INDEX1_SUPPORTED  0
#define OPTION_CIABTR_INDEX2_SUPPORTED  1
#define OPTION_CIABTR_INDEX3_SUPPORTED  1
#define OPTION_CIABTR_INDEX4_SUPPORTED  1
#define OPTION_CIABTR_INDEX5_SUPPORTED  1
#define OPTION_CIABTR_INDEX6_SUPPORTED  1
#define OPTION_CIABTR_INDEX7_SUPPORTED  1
#define OPTION_CIABTR_INDEX8_SUPPORTED  1
#define OPTION_CIABTR_INDEX9_SUPPORTED  0

#define OPTION_WRITE_TX_BUSY_DISABLED   1

#endif /* TEMPLATE_H_INCLUDED */

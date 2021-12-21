//
//  TouCAN - macOS User-Space Driver for Rusoku TouCAN USB Interfaces
//
//  Copyright (C) 2021  Uwe Vogt, UV Software, Berlin (info@mac-can.com)
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

//  A Template for CAN API V3 Testing
//
//  (§1) include the header file of the CAN API V3 C++ class of the CAN driver
#include "TouCAN.h"

//  (§2) define macro CDriverCAN with the class name of the CAN driver
#define CDriverCAN  CTouCAN

//  (§3) define macro CAN_DEVICE1 and CAN_DEVICE2 with a valid CAN channel no.
#define CAN_DEVICE1  TOUCAN_USB_CHANNEL0
#define CAN_DEVICE2  TOUCAN_USB_CHANNEL1

//  (§4) define macros for CAN Classic bit-rate settings (at least BITRATE_1M, BITRATE_500K, BITRATE_250K, BITRATE_125K)
#define BITRATE_1M(x)       do{ x.btr.frequency=50000000; x.btr.nominal.brp=5;  x.btr.nominal.tseg1=7;  x.btr.nominal.tseg2=2; x.btr.nominal.sjw=2; x.btr.nominal.sam=0; } while(0)
#define BITRATE_500K(x)     do{ x.btr.frequency=50000000; x.btr.nominal.brp=5;  x.btr.nominal.tseg1=14; x.btr.nominal.tseg2=5; x.btr.nominal.sjw=4; x.btr.nominal.sam=0; } while(0)
#define BITRATE_250K(x)     do{ x.btr.frequency=50000000; x.btr.nominal.brp=10; x.btr.nominal.tseg1=14; x.btr.nominal.tseg2=5; x.btr.nominal.sjw=4; x.btr.nominal.sam=0; } while(0)
#define BITRATE_125K(x)     do{ x.btr.frequency=50000000; x.btr.nominal.brp=20; x.btr.nominal.tseg1=15; x.btr.nominal.tseg2=5; x.btr.nominal.sjw=4; x.btr.nominal.sam=0; } while(0)

//  ($5) define macros for unsupported features in CAN Classic operation mode (at least BITRATE_800K_UNSUPPORTED, ..)
#define BITRATE_800K_UNSUPPORTED  1
#define BITRATE_5K_UNSUPPORTED  1
#define TX_ACKNOWLEDGE_UNSUPPORTED  1

//  (§6) define macros for workarounds in CAN Classic operation mode (e.g. TC01_3_WORKARAOUND)
//#define TX0x_y_WORKARAOUND  1

//  (§7) define macro CAN_FD_SUPPORTED when CAN FD operation mode is supported
#define CAN_FD_SUPPORTED  0
#if (CAN_FD_SUPPORTED != 0)

//  (§8) define macros for CAN Classic bit-rate settings (at least BITRATE_1M8M, BITRATE_500K4M, BITRATE_250K2M, BITRATE_125K1M)
#define BITRATE_1M8M(x)     do{ x.btr.frequency=0; x.btr.nominal.brp=0; x.btr.nominal.tseg1=0; x.btr.nominal.tseg2=0; x.btr.nominal.sjw=0; x.btr.nominal.sam=0; x.btr.data.brp=0; x.btr.data.tseg1=0; x.btr.data.tseg2=0; x.btr.data.sjw=0; } while(0)
#define BITRATE_500K4M(x)   do{ x.btr.frequency=0; x.btr.nominal.brp=0; x.btr.nominal.tseg1=0; x.btr.nominal.tseg2=0; x.btr.nominal.sjw=0; x.btr.nominal.sam=0; x.btr.data.brp=0; x.btr.data.tseg1=0; x.btr.data.tseg2=0; x.btr.data.sjw=0; } while(0)
#define BITRATE_250K2M(x)   do{ x.btr.frequency=0; x.btr.nominal.brp=0; x.btr.nominal.tseg1=0; x.btr.nominal.tseg2=0; x.btr.nominal.sjw=0; x.btr.nominal.sam=0; x.btr.data.brp=0; x.btr.data.tseg1=0; x.btr.data.tseg2=0; x.btr.data.sjw=0; } while(0)
#define BITRATE_125K1M(x)   do{ x.btr.frequency=0; x.btr.nominal.brp=0; x.btr.nominal.tseg1=0; x.btr.nominal.tseg2=0; x.btr.nominal.sjw=0; x.btr.nominal.sam=0; x.btr.data.brp=0; x.btr.data.tseg1=0; x.btr.data.tseg2=0; x.btr.data.sjw=0; } while(0)

//  ($9) define macros for unsupported features in CAN Classic operation mode
//#define BITRATE_SWITCHING_UNSUPPORTED  0

//  (§10) define macros for workarounds in CAN Classic operation mode (e.g. TC01_3_WORKARAOUND)
//#define TX0x_y_WORKARAOUND  1

#endif // CAN_FD_SUPPORTED
#endif // TEMPLATE_H_INCLUDED

// $Id: Template.h 1035 2021-12-21 12:03:27Z makemake $  Copyright (c) UV Software, Berlin //

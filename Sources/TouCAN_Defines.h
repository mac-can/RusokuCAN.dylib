/*
 *  CAN Interface API, Version 3 (for Rusoku TouCAN Interface)
 *
 *  Copyright (C) 2020-2021  Uwe Vogt, UV Software, Berlin (info@mac-can.com)
 *
 *  This file is part of CAN API V3.
 *
 *  CAN API V3 is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  CAN API V3 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with CAN API V3.  If not, see <http://www.gnu.org/licenses/>.
 */
 /** @addtogroup  can_api
  *  @{
  */
#ifndef CANAPI_RUSOKUCAN_H_INCLUDED
#define CANAPI_RUSOKUCAN_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/*  -----------  includes  ------------------------------------------------
 */

#include <stdint.h>                     /* C99 header for sized integer types */
#include <stdbool.h>                    /* C99 header for boolean type */


/*  -----------  options  ------------------------------------------------
 */


/*  -----------  defines  ------------------------------------------------
 */

/** @name  CAN API Interfaces
 *  @brief TouCAN USB channel no.
 *  @{ */
#define TOUCAN_USB_CHANNEL0        0    /**< Rusoku TouCAN USB Interface, Channel 0 */
#define TOUCAN_USB_CHANNEL1        1    /**< Rusoku TouCAN USB Interface, Channel 1 */
#define TOUCAN_USB_CHANNEL2        2    /**< Rusoku TouCAN USB Interface, Channel 2 */
#define TOUCAN_USB_CHANNEL3        3    /**< Rusoku TouCAN USB Interface, Channel 3 */
#define TOUCAN_USB_CHANNEL4        4    /**< Rusoku TouCAN USB Interface, Channel 4 */
#define TOUCAN_USB_CHANNEL5        5    /**< Rusoku TouCAN USB Interface, Channel 5 */
#define TOUCAN_USB_CHANNEL6        6    /**< Rusoku TouCAN USB Interface, Channel 6 */
#define TOUCAN_USB_CHANNEL7        7    /**< Rusoku TouCAN USB Interface, Channel 7 */
#define TOUCAN_BOARDS             (8)   /**< number of Rusoku TouCAN Interface boards */
// alternative defines
#define TOUCAN_USB1                TOUCAN_USB_CHANNEL0
#define TOUCAN_USB2                TOUCAN_USB_CHANNEL1
#define TOUCAN_USB3                TOUCAN_USB_CHANNEL2
#define TOUCAN_USB4                TOUCAN_USB_CHANNEL3
#define TOUCAN_USB5                TOUCAN_USB_CHANNEL4
#define TOUCAN_USB6                TOUCAN_USB_CHANNEL5
#define TOUCAN_USB7                TOUCAN_USB_CHANNEL6
#define TOUCAN_USB8                TOUCAN_USB_CHANNEL7
/** @} */

/** @name  CAN API Error Codes
 *  @brief TouCAN specific error code
 *  @{ */
#define TOUCAN_ERR_OFFSET      (-500)   /**< offset for TouCAN-specific errors */
#define TOUCAN_ERR_UNKNOWN     (-599)   /**< unknown error */
/** @} */

/** @name  CAN API Property Value
 *  @brief TouCAN parameter to be read or written
 *  @{ */
// TODO: define parameters
// ...
#define TOUCAN_MAX_BUFFER_SIZE   256U   /**< max. buffer size for CAN_GetValue/CAN_SetValue */
/** @} */


/** @name  CAN API Library ID
 *  @brief Library ID and dynamic library names
 *  @{ */
#define TOUCAN_LIB_ID            500    /**< library ID (CAN/COP API V1 compatible) */
#if defined(_WIN32) || defined (_WIN64)
 #define TOUCAN_LIB_CANLIB      "CANAL.dll"
 #define TOUCAN_LIB_WRAPPER     "u3cantou.dll"
#elif defined(__APPLE__)
 #define TOUCAN_LIB_CANLIB      "(none)"
 #define TOUCAN_LIB_WRAPPER     "libUVCANTOU.dylib"
#else
#error Platform not supported
#endif
/** @} */

/** @name  Miscellaneous
 *  @brief More or less useful stuff
 *  @{ */
#define TOUCAN_LIB_VENDOR       "Rusoku technologijos UAB, Lithuania"
#define TOUCAN_LIB_WEBSITE      "www.rusoku.com"
#define TOUCAN_LIB_HAZARD_NOTE  "Do not connect your CAN device to a real CAN network when using this program.\n" \
                                "This can damage your application."
/** @} */

#ifdef __cplusplus
}
#endif
#endif /* CANAPI_RUSOKUCAN_H_INCLUDED */
/** @}
 */
/*  ----------------------------------------------------------------------
 *  Uwe Vogt,  UV Software,  Chausseestrasse 33 A,  10115 Berlin,  Germany
 *  Tel.: +49-30-46799872,  Fax: +49-30-46799873,  Mobile: +49-170-3801903
 *  E-Mail: uwe.vogt@uv-software.de,  Homepage: http://www.uv-software.de/
 */

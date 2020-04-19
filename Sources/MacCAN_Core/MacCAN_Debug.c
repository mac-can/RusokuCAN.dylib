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
#include "MacCAN_Debug.h"

#include <stdio.h>
#include <stdarg.h>

int can_dbg_printf(FILE *file, const char *format,...) {
    int rc = 0;
#if (OPTION_MACCAN_DEBUG_LEVEL > 0)
    va_list args;
    va_start(args, format);
    rc = vfprintf(file, format, args);
    va_end(args);
#else
    if (format) { rc = 0; } /* to avoid compiler warnings */
    if (file) { rc = 0; } /* to avoid compiler warnings */
#endif
    return rc;
}

int can_dbg_func_printf(FILE *file, const char *name, const char *format,...) {
    int rc = 0;
#if (OPTION_MACCAN_INSTRUMENTATION > 0)
    fprintf(file, "%s: ", name);
    va_list args;
    va_start(args, format);
    rc = vfprintf(file, format, args);
    va_end(args);
#else
    if (format) { rc = 0; } /* to avoid compiler warnings */
    if (name) { rc = 0; } /* to avoid compiler warnings */
    if (file) { rc = 0; } /* to avoid compiler warnings */
#endif
    return rc;
}

int can_dbg_code_printf(FILE *file, int line, int level, const char *format,...) {
    int rc = 0;
#if (OPTION_MACCAN_INSTRUMENTATION > 1)
    fprintf(file, "#%i", line);
    for (int i = 0; i <= level; i++)
        fprintf(file, ".");
    va_list args;
    va_start(args, format);
    rc = vfprintf(file, format, args);
    va_end(args);
#else
    if (format) { rc = 0; } /* to avoid compiler warnings */
    if (level) { rc = 0; } /* to avoid compiler warnings */
    if (line) { rc = 0; } /* to avoid compiler warnings */
    if (file) { rc = 0; } /* to avoid compiler warnings */
#endif
    return rc;
}

#
#	TouCAN - macOS User-Space Driver for Rusoku TouCAN USB Interfaces
#
#	Copyright (C) 2020-2021  Uwe Vogt, UV Software, Berlin (info@mac-can.com)
#
#	This file is part of MacCAN-TouCAN.
#
#	MacCAN-TouCAN is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	MacCAN-TouCAN is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with MacCAN-TouCAN.  If not, see <http://www.gnu.org/licenses/>.
#
current_OS := $(shell sh -c 'uname 2>/dev/null || echo Unknown OS')
current_OS := $(patsubst CYGWIN%,Cygwin,$(current_OS))
current_OS := $(patsubst MINGW%,MinGW,$(current_OS))
current_OS := $(patsubst MSYS%,MinGW,$(current_OS))

##############################################################################

LIBRARY = libTouCAN

MAJOR = 0
MINOR = 2
PATCH = 0

VERSION = $(MAJOR).$(MINOR)
TARGET  = $(LIBRARY).$(VERSION).dylib

INSTALL_NAME = $(TARGET)
CURRENT_VERSION = $(MAJOR).$(MINOR).0
COMPATIBILITY_VERSION = $(MAJOR).1.0

INSTALL = /usr/local/lib

INCLUDE = /usr/local/include

HOME_DIR = ../..

SOURCE_DIR = $(HOME_DIR)/Sources
CANAPI_DIR = $(HOME_DIR)/Sources/CANAPI
MACCAN_DIR = $(HOME_DIR)/Sources/MacCAN_Core
TOUCAN_DIR = $(HOME_DIR)/Sources/TouCAN_Driver

##############################################################################

OBJECTS = $(OUTDIR)/TouCAN.o $(OUTDIR)/TouCAN_USB.o \
	$(OUTDIR)/MacCAN.o $(OUTDIR)/MacCAN_Debug.o $(OUTDIR)/MacCAN_Devices.o \
	$(OUTDIR)/MacCAN_IOUsbKit.o $(OUTDIR)/MacCAN_MsgQueue.o \
	$(OUTDIR)/can_btr.o

DEFINES = -DOPTION_CAN_2_0_ONLY=0 \
	-DOPTION_TOUCAN_DYLIB=1 \
	-DOPTION_CANAPI_DRIVER=1 \
	-DOPTION_CANAPI_RETVALS=0 \
	-DOPTION_CANAPI_COMPANIONS=1 \
	-DOPTION_MACCAN_LOGGER=1 \
	-DOPTION_MACCAN_MULTICHANNEL=0 \
	-DOPTION_MACCAN_PIPE_TIMEOUT=0 \
	-DOPTION_MACCAN_DEBUG_LEVEL=0 \
	-DOPTION_MACCAN_INSTRUMENTATION=0

HEADERS = -I$(SOURCE_DIR) \
	-I$(CANAPI_DIR) \
	-I$(MACCAN_DIR) \
	-I$(TOUCAN_DIR)

CFLAGS += -O2 -Wall -Wno-parentheses \
	-fno-strict-aliasing \
	$(DEFINES) \
	$(HEADERS)

CXXFLAGS += -g -Wall -Wextra -pthread \
	$(DEFINES) \
	$(HEADERS)

LIBRARIES =

LDFLAGS  += -nostartfiles -dynamiclib -fvisibility=hidden \
	-Wl,-framework -Wl,IOKit -Wl,-framework -Wl,CoreFoundation \
	-install_name $(INSTALL_NAME) \
	-current_version $(CURRENT_VERSION) \
	-compatibility_version $(COMPATIBILITY_VERSION)

##############################################################################

CXX = g++
CC = gcc
LD = g++
RM = rm -f
CP = cp -f

OUTDIR = .objects

.PHONY: info outdir


all: info outdir $(TARGET)

info:
	@echo $(CXX)" on "$(current_OS)
	@echo "Target: "$(TARGET)
	@echo "install: "$(INSTALL)

outdir:
	@mkdir -p $(OUTDIR)

clean:
	$(RM) $(TARGET) $(OUTDIR)/*.o $(OUTDIR)/*.d

install:
	@echo "Copying library file..."
	$(CP) $(TARGET) $(INSTALL)
ifeq ($(current_OS),Darwin)
	$(RM) $(INSTALL)/$(LIBRARY).dylib; ln -s $(INSTALL)/$(TARGET) $(INSTALL)/$(LIBRARY).dylib
endif
	@echo "Copying header files..."
	$(CP) $(HOME_DIR)/Sources/TouCAN.h $(INCLUDE)
	$(CP) $(HOME_DIR)/Sources/MacCAN_Core/MacCAN.h $(INCLUDE)
	$(CP) $(HOME_DIR)/Sources/CANAPI/CANAPI_Types.h $(INCLUDE)
	$(CP) $(HOME_DIR)/Sources/CANAPI/CANAPI_Defines.h $(INCLUDE)

##############################################################################

$(OUTDIR)/TouCAN.o: $(SOURCE_DIR)/TouCAN.cpp
	$(CXX) $(CXXFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/TouCAN_USB.o: $(TOUCAN_DIR)/TouCAN_USB.c
	$(CC) $(CFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/MacCAN.o: $(MACCAN_DIR)/MacCAN.cpp
	$(CXX) $(CXXFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/MacCAN_Debug.o: $(MACCAN_DIR)/MacCAN_Debug.c
	$(CC) $(CFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/MacCAN_Devices.o: $(MACCAN_DIR)/MacCAN_Devices.c
	$(CC) $(CFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/MacCAN_IOUsbKit.o: $(MACCAN_DIR)/MacCAN_IOUsbKit.c
	$(CC) $(CFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/MacCAN_MsgQueue.o: $(MACCAN_DIR)/MacCAN_MsgQueue.c
	$(CC) $(CFLAGS) -MMD -MF $*.d -o $@ -c $<

$(OUTDIR)/can_btr.o: $(CANAPI_DIR)/can_btr.c
	$(CC) $(CFLAGS) -MMD -MF $*.d -o $@ -c $<

##############################################################################

$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBRARIES)
	@echo "Target '"$@"' successfully build"

##############################################################################

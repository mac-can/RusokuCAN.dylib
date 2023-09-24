#
#	TouCAN - macOS User-Space Driver for Rusoku TouCAN USB Interfaces
#
#	Copyright (C) 2020-2022  Uwe Vogt, UV Software, Berlin (info@mac-can.com)
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

all:
	@./build_no.sh
	@echo "\033[1mBuilding MacCAN-TouCAN...\033[0m"
	$(MAKE) -C Trial $@
	$(MAKE) -C Libraries/CANAPI $@
	$(MAKE) -C Libraries/TouCAN $@
	$(MAKE) -C Utilities/can_test $@
	$(MAKE) -C Utilities/can_moni $@
#	$(MAKE) -C Tests/CANAPI $@

clean:
	$(MAKE) -C Trial $@
	$(MAKE) -C Libraries/CANAPI $@
	$(MAKE) -C Libraries/TouCAN $@
	$(MAKE) -C Utilities/can_test $@
	$(MAKE) -C Utilities/can_moni $@
#	$(MAKE) -C Tests/CANAPI $@

pristine:
	$(MAKE) -C Trial $@
	$(MAKE) -C Libraries/CANAPI $@
	$(MAKE) -C Libraries/TouCAN $@
	$(MAKE) -C Utilities/can_test $@
	$(MAKE) -C Utilities/can_moni $@
#	$(MAKE) -C Tests/CANAPI $@

install:
#	$(MAKE) -C Trial $@
	$(MAKE) -C Libraries/CANAPI $@
	$(MAKE) -C Libraries/TouCAN $@
#	$(MAKE) -C Utilities/can_test $@
#	$(MAKE) -C Utilities/can_moni $@
#	$(MAKE) -C Tests/CANAPI $@

test:
	$(MAKE) -C Trial $@

check:
	$(MAKE) -C Trial $@ 2> checker.txt

xctest:
	$(MAKE) -C Trial $@

smoketest:
	$(MAKE) -C Tests/CANAPI clean all
	./Tests/CANAPI/tou_testing --gtest_filter="SmokeTest.*"

build_no:
	@./build_no.sh
	@cat Sources/build_no.h

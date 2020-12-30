#
#	TouCAN - macOS User-Space Driver for Rusoku TouCAN USB Interfaces
#
#	Copyright (C) 2020  Uwe Vogt, UV Software, Berlin (info@mac-can.com)
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
	$(MAKE) -C Trial $@
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
	$(MAKE) -C Libraries/TouCAN $@
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
	$(MAKE) -C Libraries/CANAPI $@
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
	$(MAKE) -C Examples/can_test $@
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
	$(MAKE) -C Examples/can_moni $@

clean:
	$(MAKE) -C Trial $@
	$(MAKE) -C Libraries/TouCAN $@
	$(MAKE) -C Libraries/CANAPI $@
	$(MAKE) -C Examples/can_test $@
	$(MAKE) -C Examples/can_moni $@

install:
#	$(MAKE) -C Trial $@
	$(MAKE) -C Libraries/TouCAN $@
	$(MAKE) -C Libraries/CANAPI $@
#	$(MAKE) -C Examples/can_test $@
#	$(MAKE) -C Examples/can_moni $@

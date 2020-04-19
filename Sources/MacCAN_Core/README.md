### Creation of USB User-Space Drivers for CAN Interfaces under macOS&reg;

_Copyright &copy; 2012-2020  Uwe Vogt, UV Software, Berlin (info@mac-can.com)_

Version $Rev: 877 $

# Running CAN and CAN FD on a Mac&reg;

_Running CAN and CAN FD on a Mac_ is the mission of the MacCAN project.
The MacCAN-Core repo is not aimed at building a driver or a library.
It provides the source code of an abstraction (or rather a wrapper) of Apple´s IOUsbKit to create USB user-space drivers for CAN interfaces from various vendors under macOS.

## A Stupid Question

I´m working with the CAN bus since the late 1990th, mainly on microcontrollers
 (CANopen, SAE J1939, ISO-TP, UDS, etc.), but also on PC. See my [blog](https://www.uv-software.com/wordpress/) for the things I did.

Sometimes I tend to ask myself some stupid questions like _"Is it possible to ...?"_.
In 2010 I asked a well known search engine if it is possible to run CAN on a Mac.
I only found the request of a student, who had to do a semester work with CAN bus.
But that poor boy only owned a Mac.

## MacCAN - macOS Driver for PCAN-USB Interfaces and more

In 2012 I´ve started with the development of an OS X user-space driver for my PEAK CAN to USB dongle.
Many thanks to Uwe Wilhelm, CEO of PEAK-System Technik GmbH, who had supported me with technical information and several hardware.

### PCBUSB Library

The _PCBUSB_ library realizes a USB user-space driver under macOS for PCAN&reg; USB interfaces from PEAK-System Technik.
See the [MacCAN](https://www.mac-can.com) website for further information, downloads and links.

The library supports up to 8 PCAN-USB and PCAN-USB FD devices.
It offers an easy to use API that is almost compatible to PEAK´s PCANBasic DLL on Windows.
Standard CAN frames (11-bit identifier) as well as extended CAN frames (29-bit identifier) are supported.
PCAN-USB FD devices can be operated in CAN 2.0 mode and in CAN FD mode.

The library comes with an Objective-C wrapper and a demo App; see https://youtu.be/v0U_WN7s3ao/ \
Furthermore, it can be used with the Qt&reg; Serial Bus API on a Mac; see https://doc.qt.io/qt-5/qtserialbus-index.html

The PCBUSB library is closed source, please note the copyright and license agreements.

### TouCAN USB Interfaces from Rusoku

In 2020 I´ve received the request from Gediminas Simanskis, CEO of Rusoku technologijos UAB, Lithuania, if it is possible to roll out the MacCAN project to their TouCAN USB devices.
See [Rusoku](https://www.rusoku.com)´s website for the products and the services they offer.

The driver for TouCAN USB interfaces from Rusoku is the first driver implementation based on MacCAN-Core.
And it is open source;
goto https://github.com/mac-can/RusokuCAN/.

## HowTo

[To Be Continued]

## This and That

### SVN Repo

The MacCAN-Core sources are maintained in a SVN repo to synchronized them between the different MacCAN driver repos via Git SVN bridge.

### License

MacCAN-Core is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MacCAN-Core is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with MacCAN-Core.  If not, see <http://www.gnu.org/licenses/>.

### Trademarks

Mac and macOS are trademarks of Apple Inc., registered in the U.S. and other countries. \
PCAN is a registered trademark of PEAK-System Technik GmbH, Darmstadt, Germany. \
Qt is a registered trademark of The Qt Company Ltd. and its subsidiaries.

### Contact

Uwe Vogt \
UV Software \
Chausseestrasse 33a \
10115 Berlin \
Germany

E-Mail: mailto://info@mac.can.com \
Internet: https://www.mac-can.com

##### *Have a lot of fun!*

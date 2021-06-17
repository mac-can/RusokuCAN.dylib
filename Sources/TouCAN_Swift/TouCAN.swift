//  SPDX-License-Identifier: GPL-3.0-or-later
//
//  TouCAN - macOS User-Space Driver for Rusoku TouCAN USB Interfaces
//
//  Copyright (c) 2020-2021 Uwe Vogt, UV Software, Berlin (info@mac-can.com)
//  All rights reserved.
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
//  along with MacCAN-TouCAN.  If not, see <http://www.gnu.org/licenses/>.
//
import Foundation

public enum TouCanChannel: Int32, CaseIterable {
    case Channel0 = 0
    case Channel1, Channel2, Channel3,
         Channel4, Channel5, Channel6,
         Channel7
    public var description: String {
        return "TouCAN-USB\(self.rawValue + 1)"
    }
}

public class TouCAN: CanApi {
    // properties and methods from class CanApi
}

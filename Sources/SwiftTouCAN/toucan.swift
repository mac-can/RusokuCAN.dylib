import CTouCAN
import Foundation

public class TouCAN {

    static let InvalidHandle: Int32 = -1

    public enum Error: Int32, Swift.Error {
        case NOERROR              = 0 /**< no error! */
        case BOFF                = -1 /**< CAN - busoff status */
        case EWRN                = -2 /**< CAN - error warning status */
        case BERR                = -3 /**< CAN - bus error */
        case OFFLINE             = -9 /**< CAN - not started */
        case ONLINE              = -8 /**< CAN - already started */
        case MSG_LST            = -10 /**< CAN - message lost */
        case LEC_STUFF          = -11 /**< LEC - stuff error */
        case LEC_FORM           = -12 /**< LEC - form error */
        case LEC_ACK            = -13 /**< LEC - acknowledge error */
        case LEC_BIT1           = -14 /**< LEC - recessive bit error */
        case LEC_BIT0           = -15 /**< LEC - dominant bit error */
        case LEC_CRC            = -16 /**< LEC - checksum error */
        case TX_BUSY            = -20 /**< USR - transmitter busy */
        case RX_EMPTY           = -30 /**< USR - receiver empty */
        case ERR_FRAME          = -40 /**< USR - error frame */
        case TIMEOUT            = -50 /**< USR - time-out */
        case RESOURCE           = -90 /**< USR - resource allocation */
        case BAUDRATE           = -91 /**< USR - illegal baudrate */
        case HANDLE             = -92 /**< USR - illegal handle */
        case ILLPARA            = -93 /**< USR - illegal parameter */
        case NULLPTR            = -94 /**< USR - null-pointer assignment */
        case NOTINIT            = -95 /**< USR - not initialized */
        case YETINIT            = -96 /**< USR - already initialized */
        case LIBRARY            = -97 /**< USR - illegal library */
        case NOTSUPP            = -98 /**< USR - not supported */
        case FATAL              = -99 /**< USR - other errors */
        case VENDOR            = -100 /**< USR - vendor specific */

        case UNKNOWN           = -999
    }

    public struct Mode: OptionSet {
        public init(rawValue: UInt8) {
            self.rawValue = rawValue
        }
        public let rawValue: UInt8

        public static let  FDOE              = Mode(rawValue: 0x80) /**< CAN FD operation enable/disable */
        public static let  BRSE              = Mode(rawValue: 0x40) /**< bit-rate switch enable/disable */
        public static let  NISO              = Mode(rawValue: 0x20) /**< Non-ISO CAN FD enable/disable */
        public static let  SHRD              = Mode(rawValue: 0x10) /**< shared access enable/disable */
        public static let  NXTD              = Mode(rawValue: 0x08) /**< extended format disable/enable */
        public static let  NRTR              = Mode(rawValue: 0x04) /**< remote frames disable/enable */
        public static let  ERR               = Mode(rawValue: 0x02) /**< error frames enable/disable */
        public static let  MON               = Mode(rawValue: 0x01) /**< monitor mode enable/disable */
        public static let  DEFAULT           = Mode([])             /**< CAN 2.0 operation mode */
    }

    /// A CAN Message
    public struct Message: Codable, Equatable {

        /// CAN Identifier (aka Arbitration ID aka Header)
        public let id: UInt32
        public let xtd: Bool
        public let rtr: Bool
        /// CANFD format
        public let fdf: Bool
        /// CANFD Bitrate Switching
        public let brs: Bool
        /// CANFD Error State Indicator
        public let esi: Bool
        /// Status Message
        public let sts: Bool
        /// DLC or, for CANFD, length-code
        public let dlc: UInt8
        public let data: [UInt8]
        /// Time since booting the micro controller on the hardware
        public let timestamp: TimeInterval

        public var length: UInt8 {
            guard self.fdf else { return self.dlc }
            switch self.dlc {
                case 9: return 12
                case 10: return 16
                case 11: return 20
                case 12: return 24
                case 13: return 32
                case 14: return 48
                case 15: return 64
                default: return self.dlc
            }
        }

        public init(id: UInt32, data: [UInt8]) {
            precondition(data.count > 0 && data.count < 9, "Unsupported data length. Data length should be between 1 and 8")
            self.id = id
            self.xtd = false
            self.rtr = false
            self.fdf = false
            self.brs = false
            self.esi = false
            self.sts = false
            self.dlc = max(8 as UInt8, UInt8(data.count))
            var data = data
            while data.count < 8 {
                data = data + [0x55]
            }
            self.data = data
            self.timestamp = 0
        }

        init(from cm: can_message_t) {
            self.id = cm.id
            self.xtd = cm.xtd != 0
            self.rtr = cm.rtr != 0
            self.fdf = cm.fdf != 0
            self.brs = cm.brs != 0
            self.esi = cm.esi != 0
            self.sts = cm.sts != 0
            self.dlc = cm.dlc
            switch self.dlc {
                case 0: self.data = []
                case 1: self.data = [cm.data.0]
                case 2: self.data = [cm.data.0, cm.data.1]
                case 3: self.data = [cm.data.0, cm.data.1, cm.data.2]
                case 4: self.data = [cm.data.0, cm.data.1, cm.data.2, cm.data.3]
                case 5: self.data = [cm.data.0, cm.data.1, cm.data.2, cm.data.3, cm.data.4]
                case 6: self.data = [cm.data.0, cm.data.1, cm.data.2, cm.data.3, cm.data.4, cm.data.5]
                case 7: self.data = [cm.data.0, cm.data.1, cm.data.2, cm.data.3, cm.data.4, cm.data.5, cm.data.6]
                case 8: self.data = [cm.data.0, cm.data.1, cm.data.2, cm.data.3, cm.data.4, cm.data.5, cm.data.6, cm.data.7]
                default:
                    preconditionFailure("CANFD frames not yet supported. TODO: Use Swift's mirror functionality to create a tuple-iterator")
            }
            self.timestamp = Double(cm.timestamp.tv_sec) + Double( cm.timestamp.tv_nsec / 1000000000)
        }

        var cm: can_message_t {
            var cm = can_message_t()
            cm.id = self.id
            cm.dlc = self.dlc
            switch self.dlc {
                case 0: cm.data = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
                case 1: cm.data = (data[0], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
                case 2: cm.data = (data[0], data[1], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
                case 3: cm.data = (data[0], data[1], data[2], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
                case 4: cm.data = (data[0], data[1], data[2], data[3], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
                case 5: cm.data = (data[0], data[1], data[2], data[3], data[4], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
                case 6: cm.data = (data[0], data[1], data[2], data[3], data[4], data[5], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
                case 7: cm.data = (data[0], data[1], data[2], data[3], data[4], data[5], data[6], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
                case 8: cm.data = (data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
                default:
                    preconditionFailure("CANFD frames not yet supported. TODO: Create a tuple-iterator")
            }
            return cm
        }
    }

    public var hardwareVersion: String {
        guard self.handle != Self.InvalidHandle else { return "???" }
        guard let cString = can_hardware(self.handle) else { return "???" }
        return String(cString: cString)
    }

    public var softwareVersion: String {
        guard self.handle != Self.InvalidHandle else { return "???" }
        guard let cString = can_software(self.handle) else { return "???" }
        return String(cString: cString)
    }

    private lazy var handle: Int32 = Self.InvalidHandle

    public init() { }

    /// Connect to the (first) CAN adapter with the given `mode` and `baudrate`.
    public func connect(channel: UInt8 = 0, mode: Mode = .DEFAULT, baudrate: Int) throws {
        let handleOrResult = can_init(Int32(channel), mode.rawValue, nil)
        guard handleOrResult == CANERR_NOERROR else { throw Error(rawValue: handleOrResult) ?? Error.UNKNOWN }

        guard var bitrate = can_bitrate_t(baudrate: baudrate) else { throw Error.BAUDRATE }
        self.handle = handleOrResult
        let result = can_start(handle, &bitrate)
        guard result == CANERR_NOERROR else { throw Error(rawValue: handleOrResult) ?? Error.UNKNOWN }
    }

    /// Read a message. The timeout is given in milliseconds with `0xFFFF` being _forever_.
    public func readMessage(timeout: UInt16 = 0xFFFF) throws -> Message {

        var message = can_message_t()
        let result = can_read(self.handle, &message, timeout)
        guard result == CANERR_NOERROR else { throw Error(rawValue: result) ?? Error.UNKNOWN }
        return Message(from: message)
    }

    /// Write a message. THe timeout is given in milliseconds with `0xFFFF` being _forever_.
    public func writeMessage(_ message: Message, timeout: UInt16 = 0xFFFF) throws {

        precondition(message.dlc > 0 && message.data.count > 0, "CAN Message empty")
        var cm = message.cm
        let result = can_write(self.handle, &cm, timeout)
        guard result == CANERR_NOERROR else { throw Error(rawValue: result) ?? Error.UNKNOWN }
    }

    /// Signal a waiting operation (read, write, etc.) to exit immediately.
    public func kill() throws {

        let result = can_kill(self.handle)
        guard result == CANERR_NOERROR else { throw Error(rawValue: result) ?? Error.UNKNOWN }
    }

    /// Returns the status register.
    public func status() throws -> UInt8 {

        var status: UInt8 = 0
        let result = can_status(self.handle, &status)
        guard result == CANERR_NOERROR else { throw Error(rawValue: result) ?? Error.UNKNOWN }
        return status
    }

    /// Returns the bus load and status register.
    public func busload() throws -> (busload: Double, status: UInt8) {

        var status: UInt8 = 0
        var percent: UInt8 = 0
        let result = can_busload(self.handle, &percent, &status)
        guard result == CANERR_NOERROR else { throw Error(rawValue: result) ?? Error.UNKNOWN }
        let busload = Double(percent) / 100.0
        return (busload: busload, status: status)
    }

    /*
    public func setMessageFilter(id: UInt32) throws {

        let property = id > 0x7FF ? CANPROP_SET_FLT_29BIT_CODE : CANPROP_SET_FLT_11BIT_CODE
        let result = can_property(self.handle, UInt32(property), id, 4)
    }
    */

    deinit {
        guard self.handle != Self.InvalidHandle else { return }
        can_exit(self.handle)
    }
}

private extension can_bitrate_t {

    init?(baudrate: Int) {
        switch baudrate {
            case 1000000:
                self.init(index: CANBTR_INDEX_1M)
            case 500000:
                self.init(index: CANBTR_INDEX_500K)
            case 250000:
                self.init(index: CANBTR_INDEX_250K)
            case 125000:
                self.init(index: CANBTR_INDEX_125K)
            case 100000:
                self.init(index: CANBTR_INDEX_100K)
            case 50000:
                self.init(index: CANBTR_INDEX_50K)
            case 20000:
                self.init(index: CANBTR_INDEX_20K)
            case 10000:
                self.init(index: CANBTR_INDEX_10K)
            default:
                return nil
        }
    }

}

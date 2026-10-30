#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace voltguard::modbus {

// Day 3: frame parsing/serialization and higher-level Modbus validation.
enum class FunctionCode : std::uint8_t {
    ReadCoils = 0x01,
    ReadDiscreteInputs = 0x02,
    ReadHoldingRegisters = 0x03,
    ReadInputRegisters = 0x04,
    WriteSingleCoil = 0x05,
    WriteSingleRegister = 0x06,
    ReadExceptionStatus = 0x07,
    Diagnostics = 0x08,
    GetCommEventCounter = 0x0B,
    GetCommEventLog = 0x0C,
    WriteMultipleCoils = 0x0F,
    WriteMultipleRegisters = 0x10,
    ReportServerId = 0x11,
    MaskWriteRegister = 0x16,
    ReadFifoQueue = 0x18,
    EncapsulatedInterfaceTransport = 0x2B
};

struct ModbusFrame {
    bool tcp = false;
    bool request = true;
    bool exception = false;

    std::uint16_t transactionId = 0;
    std::uint16_t protocolId = 0;
    std::uint16_t length = 0;

    std::uint8_t unitId = 0;
    std::uint8_t functionCode = 0;

    std::uint16_t startAddress = 0;
    std::uint16_t quantity = 0;
    std::uint16_t value = 0;

    std::vector<std::uint8_t> payload;
    std::vector<std::uint16_t> registers;
    std::uint8_t exceptionCode = 0;

    std::string error;
};

class ModbusParser {
public:
    static bool readUint16Be(const std::vector<std::uint8_t>& data,
                             std::size_t offset,
                             std::uint16_t& value) {
        if (offset >= data.size() || data.size() - offset < 2) {
            return false;
        }

        value = static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(data[offset]) << 8) |
            static_cast<std::uint16_t>(data[offset + 1]));
        return true;
    }

    static void appendUint16Be(std::vector<std::uint8_t>& data, std::uint16_t value) {
        data.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFFu));
        data.push_back(static_cast<std::uint8_t>(value & 0xFFu));
    }

    static std::uint16_t crc16Modbus(const std::vector<std::uint8_t>& data) {
        std::uint16_t crc = 0xFFFFu;
        for (const std::uint8_t byte : data) {
            crc ^= byte;
            for (int bit = 0; bit < 8; ++bit) {
                if ((crc & 0x0001u) != 0u) {
                    crc = static_cast<std::uint16_t>((crc >> 1) ^ 0xA001u);
                } else {
                    crc = static_cast<std::uint16_t>(crc >> 1);
                }
            }
        }
        return crc;
    }

    static bool isValidRtuFrame(const std::vector<std::uint8_t>& frame) {
        if (frame.size() < 4) {
            return false;
        }

        const std::size_t crcOffset = frame.size() - 2;
        const std::uint16_t expectedCrc = static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(frame[crcOffset]) |
            (static_cast<std::uint16_t>(frame[crcOffset + 1]) << 8));
        const std::vector<std::uint8_t> data(frame.begin(), frame.begin() + crcOffset);
        return crc16Modbus(data) == expectedCrc;
    }

    static bool isSupportedFunction(std::uint8_t functionCode) {
        switch (functionCode) {
            case static_cast<std::uint8_t>(FunctionCode::ReadCoils):
            case static_cast<std::uint8_t>(FunctionCode::ReadDiscreteInputs):
            case static_cast<std::uint8_t>(FunctionCode::ReadHoldingRegisters):
            case static_cast<std::uint8_t>(FunctionCode::ReadInputRegisters):
            case static_cast<std::uint8_t>(FunctionCode::WriteSingleCoil):
            case static_cast<std::uint8_t>(FunctionCode::WriteSingleRegister):
            case static_cast<std::uint8_t>(FunctionCode::ReadExceptionStatus):
            case static_cast<std::uint8_t>(FunctionCode::Diagnostics):
            case static_cast<std::uint8_t>(FunctionCode::GetCommEventCounter):
            case static_cast<std::uint8_t>(FunctionCode::GetCommEventLog):
            case static_cast<std::uint8_t>(FunctionCode::WriteMultipleCoils):
            case static_cast<std::uint8_t>(FunctionCode::WriteMultipleRegisters):
            case static_cast<std::uint8_t>(FunctionCode::ReportServerId):
            case static_cast<std::uint8_t>(FunctionCode::MaskWriteRegister):
            case static_cast<std::uint8_t>(FunctionCode::ReadFifoQueue):
            case static_cast<std::uint8_t>(FunctionCode::EncapsulatedInterfaceTransport):
                return true;
            default:
                return false;
        }
    }

    static bool parseFrame(const std::vector<std::uint8_t>& data, ModbusFrame& frame) {
        frame = ModbusFrame{};

        if (data.empty()) {
            frame.error = "empty frame";
            return false;
        }

        if (data.size() >= 7 && data[2] == 0x00 && data[3] == 0x00) {
            return parseTcpFrame(data, frame);
        }

        return parseRtuFrame(data, frame);
    }

    static bool parseRtuFrame(const std::vector<std::uint8_t>& data, ModbusFrame& frame) {
        frame = ModbusFrame{};
        if (data.size() < 4) {
            frame.error = "RTU frame too short";
            return false;
        }

        if (!isValidRtuFrame(data)) {
            frame.error = "invalid RTU CRC";
            return false;
        }

        frame.tcp = false;
        frame.request = true;
        frame.unitId = data[0];
        frame.functionCode = data[1];

        if (!isSupportedFunction(frame.functionCode) && (frame.functionCode & 0x80u) == 0u) {
            frame.error = "unsupported function code";
            return false;
        }

        const std::vector<std::uint8_t> payload(data.begin() + 1, data.begin() + data.size() - 2);
        frame.payload = payload;
        return decodePayload(frame.functionCode, payload, frame);
    }

    static bool parseTcpFrame(const std::vector<std::uint8_t>& data, ModbusFrame& frame) {
        frame = ModbusFrame{};
        if (data.size() < 8) {
            frame.error = "TCP frame too short";
            return false;
        }

        std::uint16_t transactionIdValue = 0;
        std::uint16_t protocolIdValue = 0;
        std::uint16_t lengthValue = 0;
        if (!readUint16Be(data, 0, transactionIdValue) ||
            !readUint16Be(data, 2, protocolIdValue) ||
            !readUint16Be(data, 4, lengthValue)) {
            frame.error = "invalid TCP header";
            return false;
        }

        frame.tcp = true;
        frame.transactionId = transactionIdValue;
        frame.protocolId = protocolIdValue;
        frame.length = lengthValue;
        frame.unitId = data[6];
        frame.functionCode = data[7];

        if (!isSupportedFunction(frame.functionCode) && (frame.functionCode & 0x80u) == 0u) {
            frame.error = "unsupported function code";
            return false;
        }

        const std::size_t expectedLength = static_cast<std::size_t>(6u + lengthValue);
        if (data.size() < expectedLength) {
            frame.error = "TCP frame length mismatch";
            return false;
        }

        const std::vector<std::uint8_t> payload(data.begin() + 7, data.begin() + expectedLength);
        frame.payload = payload;
        return decodePayload(frame.functionCode, payload, frame);
    }

    static bool encodeRtuFrame(const ModbusFrame& frame, std::vector<std::uint8_t>& output) {
        output.clear();

        if (frame.functionCode == 0u) {
            return false;
        }

        output.push_back(frame.unitId);
        output.push_back(frame.functionCode);

        switch (frame.functionCode) {
            case static_cast<std::uint8_t>(FunctionCode::ReadCoils):
            case static_cast<std::uint8_t>(FunctionCode::ReadDiscreteInputs):
            case static_cast<std::uint8_t>(FunctionCode::ReadHoldingRegisters):
            case static_cast<std::uint8_t>(FunctionCode::ReadInputRegisters):
                appendUint16Be(output, frame.startAddress);
                appendUint16Be(output, frame.quantity);
                break;
            case static_cast<std::uint8_t>(FunctionCode::WriteSingleCoil):
            case static_cast<std::uint8_t>(FunctionCode::WriteSingleRegister):
                appendUint16Be(output, frame.startAddress);
                appendUint16Be(output, frame.value);
                break;
            case static_cast<std::uint8_t>(FunctionCode::WriteMultipleRegisters):
            case static_cast<std::uint8_t>(FunctionCode::WriteMultipleCoils):
                appendUint16Be(output, frame.startAddress);
                appendUint16Be(output, frame.quantity);
                output.push_back(static_cast<std::uint8_t>(frame.payload.size()));
                output.insert(output.end(), frame.payload.begin(), frame.payload.end());
                break;
            default:
                if (!frame.payload.empty()) {
                    output.insert(output.end(), frame.payload.begin(), frame.payload.end());
                }
                break;
        }

        const std::uint16_t crc = crc16Modbus(output);
        output.push_back(static_cast<std::uint8_t>(crc & 0xFFu));
        output.push_back(static_cast<std::uint8_t>((crc >> 8) & 0xFFu));
        return true;
    }

    static bool encodeTcpFrame(const ModbusFrame& frame, std::vector<std::uint8_t>& output) {
        output.clear();
        if (frame.functionCode == 0u) {
            return false;
        }

        std::vector<std::uint8_t> payload;
        payload.push_back(frame.functionCode);

        switch (frame.functionCode) {
            case static_cast<std::uint8_t>(FunctionCode::ReadCoils):
            case static_cast<std::uint8_t>(FunctionCode::ReadDiscreteInputs):
            case static_cast<std::uint8_t>(FunctionCode::ReadHoldingRegisters):
            case static_cast<std::uint8_t>(FunctionCode::ReadInputRegisters):
                appendUint16Be(payload, frame.startAddress);
                appendUint16Be(payload, frame.quantity);
                break;
            case static_cast<std::uint8_t>(FunctionCode::WriteSingleCoil):
            case static_cast<std::uint8_t>(FunctionCode::WriteSingleRegister):
                appendUint16Be(payload, frame.startAddress);
                appendUint16Be(payload, frame.value);
                break;
            case static_cast<std::uint8_t>(FunctionCode::WriteMultipleRegisters):
            case static_cast<std::uint8_t>(FunctionCode::WriteMultipleCoils):
                appendUint16Be(payload, frame.startAddress);
                appendUint16Be(payload, frame.quantity);
                payload.push_back(static_cast<std::uint8_t>(frame.payload.size()));
                payload.insert(payload.end(), frame.payload.begin(), frame.payload.end());
                break;
            default:
                if (!frame.payload.empty()) {
                    payload.insert(payload.end(), frame.payload.begin(), frame.payload.end());
                }
                break;
        }

        output.push_back(static_cast<std::uint8_t>((frame.transactionId >> 8) & 0xFFu));
        output.push_back(static_cast<std::uint8_t>(frame.transactionId & 0xFFu));
        output.push_back(0x00u);
        output.push_back(0x00u);
        output.push_back(static_cast<std::uint8_t>((static_cast<std::uint16_t>(payload.size() + 1u) >> 8) & 0xFFu));
        output.push_back(static_cast<std::uint8_t>(static_cast<std::uint16_t>(payload.size() + 1u) & 0xFFu));
        output.push_back(frame.unitId);
        output.insert(output.end(), payload.begin(), payload.end());
        return true;
    }

    static bool isValidTcpFrame(const std::vector<std::uint8_t>& data) {
        if (data.size() < 8) {
            return false;
        }

        const std::uint16_t length = static_cast<std::uint16_t>((static_cast<std::uint16_t>(data[4]) << 8) |
                                                               static_cast<std::uint16_t>(data[5]));
        return data.size() == static_cast<std::size_t>(6u + length);
    }

private:
    static bool decodePayload(std::uint8_t functionCode, const std::vector<std::uint8_t>& payload, ModbusFrame& frame) {
        if (payload.empty()) {
            frame.error = "empty payload";
            return false;
        }

        if ((functionCode & 0x80u) != 0u) {
            if (payload.size() != 2u) {
                frame.error = "invalid exception response";
                return false;
            }

            frame.exception = true;
            frame.exceptionCode = payload[1];
            frame.request = false;
            return true;
        }

        switch (functionCode) {
            case static_cast<std::uint8_t>(FunctionCode::ReadCoils):
            case static_cast<std::uint8_t>(FunctionCode::ReadDiscreteInputs):
            case static_cast<std::uint8_t>(FunctionCode::ReadHoldingRegisters):
            case static_cast<std::uint8_t>(FunctionCode::ReadInputRegisters): {
                if (payload.size() == 5u) {
                    frame.request = true;
                    if (!readUint16Be(payload, 1, frame.startAddress) ||
                        !readUint16Be(payload, 3, frame.quantity)) {
                        frame.error = "invalid read request";
                        return false;
                    }
                    return true;
                }

                if (payload.size() >= 2u) {
                    const std::size_t expectedLength = 2u + static_cast<std::size_t>(payload[1]);
                    if (payload.size() == expectedLength) {
                        frame.request = false;
                        frame.payload = payload;
                        for (std::size_t i = 2; i + 1 < payload.size(); i += 2) {
                            std::uint16_t value = 0;
                            if (!readUint16Be(payload, i, value)) {
                                frame.error = "invalid read response";
                                return false;
                            }
                            frame.registers.push_back(value);
                        }
                        return true;
                    }
                }

                frame.error = "invalid read payload";
                return false;
            }

            case static_cast<std::uint8_t>(FunctionCode::WriteSingleCoil):
            case static_cast<std::uint8_t>(FunctionCode::WriteSingleRegister):
                if (payload.size() == 5u) {
                    frame.request = true;
                    if (!readUint16Be(payload, 1, frame.startAddress) ||
                        !readUint16Be(payload, 3, frame.value)) {
                        frame.error = "invalid single-write payload";
                        return false;
                    }
                    return true;
                }
                break;

            case static_cast<std::uint8_t>(FunctionCode::WriteMultipleCoils):
            case static_cast<std::uint8_t>(FunctionCode::WriteMultipleRegisters): {
                if (payload.size() >= 5u) {
                    if (payload.size() == 5u) {
                        frame.request = false;
                        if (!readUint16Be(payload, 1, frame.startAddress) ||
                            !readUint16Be(payload, 3, frame.quantity)) {
                            frame.error = "invalid multi-write response";
                            return false;
                        }
                        return true;
                    }

                    frame.request = true;
                    if (!readUint16Be(payload, 1, frame.startAddress) ||
                        !readUint16Be(payload, 3, frame.quantity)) {
                        frame.error = "invalid multi-write request";
                        return false;
                    }
                    return true;
                }
                break;
            }

            default:
                break;
        }

        frame.request = true;
        return true;
    }
};

}  // namespace voltguard::modbus

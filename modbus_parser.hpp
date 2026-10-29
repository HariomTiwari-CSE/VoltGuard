#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace voltguard::modbus {

// Day 1: project foundation for Modbus parsing.
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
};

}  // namespace voltguard::modbus

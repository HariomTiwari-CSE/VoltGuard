#pragma once

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

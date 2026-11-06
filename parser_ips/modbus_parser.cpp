#include "modbus_parser.hpp"
uint16_t ModbusParser::bytesToUint16(
    uint8_t high,
    uint8_t low)
{
    return (static_cast<uint16_t>(high) << 8) | low;
}

"
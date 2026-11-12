#include "modbus_parser.hpp"
uint16_t ModbusParser::bytesToUint16(
    uint8_t high,
    uint8_t low)
{
    return (static_cast<uint16_t>(high) << 8) | low;
}
bool ModbusParser::parseFrame(
    const std::vector<uint8_t>& packet,
    ModbusFrame& frame)
{
        if (frame.function_code == 0x06)
    {
        if (packet.size() < 12)
        {
            return false;
        }

        frame.register_address =
            bytesToUint16(packet[8], packet[9]);

        frame.register_value =
            bytesToUint16(packet[10], packet[11]);

        return true;
    }
        if (frame.function_code == 0x10)
    {
        if (packet.size() < 13)
        {
            return false;
        }

        frame.register_address =
            bytesToUint16(packet[8], packet[9]);

        frame.register_count =
            bytesToUint16(packet[10], packet[11]);

        uint8_t byte_count = packet[12];
"
#include <iostream>
#include <vector>
#include "modbus_parser.hpp"

int main() {
    std::vector<std::uint8_t> frame = {0x11, 0x03, 0x00, 0x00, 0x00, 0x02};
    auto crc = voltguard::modbus::ModbusParser::crc16Modbus({0x11,0x03,0x00,0x00,0x00,0x02});
    frame.push_back(static_cast<std::uint8_t>(crc & 0xFFu));
    frame.push_back(static_cast<std::uint8_t>((crc >> 8) & 0xFFu));
    voltguard::modbus::ModbusFrame parsed;
    bool ok = voltguard::modbus::ModbusParser::parseFrame(frame, parsed);
    std::cout << ok << " " << parsed.error << "\n";
    return ok ? 0 : 1;
}

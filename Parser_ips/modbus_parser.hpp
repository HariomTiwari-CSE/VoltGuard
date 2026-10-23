#ifndef MODBUS_PARSER_HPP
#define MODBUS_PARSER_HPP

#include <vector>
#include <cstdint>
#include <string>

struct ParsedCommand {
    bool valid = false;
    uint16_t transaction_id = 0;
    uint8_t unit_id = 0;
    uint8_t function_code = 0;
    double target_rpm = 0.0;
    double valve_pct = 0.0;
    std::string error_message = "";
};

class ModbusParser {
public:
    static ParsedCommand parse_hex_frame(const std::vector<uint8_t>& frame);
    static ParsedCommand parse_hex_string(const std::string& hex_str);
};

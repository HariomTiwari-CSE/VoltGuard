#include "modbus_parser.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace voltguard {
namespace ips {

struct InspectionResult {
    bool parsed = false;
    bool allowed = false;
    modbus::ModbusFrame frame;
    std::vector<std::string> alerts;
};
class ModbusIpsEngine {
public:
    InspectionResult inspectRtuFrame(
        const std::vector<std::uint8_t>& bytes) const {
        InspectionResult result;

        if (bytes.size() > 256u) {
            addViolation(result, "RTU frame exceeds the 256-byte limit");
            return result;
        }

        if (!modbus::ModbusParser::parseRtuFrame(bytes, result.frame)) {
            addViolation(result, "Invalid RTU frame: " + result.frame.error);
            return result;
        }

        validateParsedFrame(result);
        return result;
    }

    InspectionResult inspectTcpFrame(
        const std::vector<std::uint8_t>& bytes) const {
        InspectionResult result;

        if (bytes.size() < 8u) {
            addViolation(result, "TCP frame is too short");
            return result;
        }

        if (bytes.size() > 260u) {
            addViolation(result, "TCP frame exceeds the 260-byte limit");
            return result;
        }

        if (bytes[2] != 0u || bytes[3] != 0u) {
            addViolation(result, "Modbus TCP protocol ID must be zero");
            return result;
        }

        if (!modbus::ModbusParser::isValidTcpFrame(bytes)) {
            addViolation(result, "Modbus TCP length does not match the frame");
            return result;
        }

        if (!modbus::ModbusParser::parseTcpFrame(bytes, result.frame)) {
            addViolation(result, "Invalid Modbus TCP frame: " + result.frame.error);
            return result;
        }

        validateParsedFrame(result);
        return result;
    }

private:
    static void addViolation(InspectionResult& result,
                             const std::string& message) {
        result.allowed = false;
        result.alerts.push_back(message);
    }

    static bool isWriteFunction(std::uint8_t functionCode) {
        return functionCode == 0x05u ||
               functionCode == 0x06u ||
               functionCode == 0x0Fu ||
               functionCode == 0x10u;
    }

    static std::uint16_t maximumQuantity(std::uint8_t functionCode) {
        switch (functionCode) {
            case 0x01u:
            case 0x02u:
                return 2000u;
            case 0x03u:
            case 0x04u:
                return 125u;
            case 0x0Fu:
                return 1968u;
            case 0x10u:
                return 123u;
            default:
                return 0u;
        }
    }

    static bool hasAddressRange(std::uint8_t functionCode) {
        return functionCode == 0x01u || functionCode == 0x02u ||
               functionCode == 0x03u || functionCode == 0x04u ||
               functionCode == 0x05u || functionCode == 0x06u ||
               functionCode == 0x0Fu || functionCode == 0x10u;
    }
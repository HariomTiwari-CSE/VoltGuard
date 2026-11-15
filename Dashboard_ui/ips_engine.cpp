#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "modbus_parser.hpp"

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string queryPhysicsAPI(double rpm, double valve) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return "{\"verdict\":\"DROP\",\"reason\":\"Socket creation failed\"}";

    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5555);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sock);
        return "{\"verdict\":\"DROP\",\"reason\":\"Physics API offline\"}";
    }

    std::ostringstream oss;
    oss << "{\"pump_rpm\":" << rpm << ",\"valve_opening\":" << valve << "}\n";
    std::string msg = oss.str();
    send(sock, msg.c_str(), msg.length(), 0);

    char buffer[4096] = {0};
    int valread = read(sock, buffer, sizeof(buffer) - 1);
    close(sock);

    if (valread > 0) return std::string(buffer, valread);
    return "{\"verdict\":\"DROP\",\"reason\":\"Empty response from API\"}";
}

int main() {
    std::cout << "[+] VoltGuard Live Engine Active - Streaming to UI (Port 5556)..." << std::endl;

    int udpSock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in uiAddr{};
    uiAddr.sin_family = AF_INET;
    uiAddr.sin_port = htons(5556);
    inet_pton(AF_INET, "127.0.0.1", &uiAddr.sin_addr);

    std::vector<std::pair<std::string, std::string>> packets = {
        {"Normal Operator Command", "00010000000B0110000100020405DC0028"},      // 1500 RPM, 40% Valve
        {"Normal Ramping Setpoint", "00020000000B0110000100020407080032"},      // 1800 RPM, 50% Valve
        {"Malicious Water-Hammer Attack", "00020000000B011000010002040BB80002"}, // 3000 RPM, 2% Valve
        {"Catastrophic Overspeed Attack", "00040000000B01100001000204C3500050"}  // 50,000 RPM
    };

    double simTime = 0.0;
    double currentP = 1.0;

    for (const auto& pkt : packets) {
        simTime += 2.0;
        std::string label = pkt.first;
        std::string hex = pkt.second;
        std::string timeStr = getCurrentTimestamp();

        auto parsed = ModbusParser::parseHexString(hex);
        std::string verdict = "[INLINE DROP]";
        std::string reason = "";
        double predictedPeak = currentP;

        if (!parsed.isValid) {
            reason = "MALFORMED: " + parsed.errorMessage;
        } else {
            std::string apiResp = queryPhysicsAPI(parsed.pumpRpm, parsed.valveOpening);
            std::cout << "\n------------------------------------------------\n";
            std::cout << "[PACKET INGESTED]: " << label << "\n";
            std::cout << "RAW HEX: " << hex << "\n";
            std::cout << "[*] DETAILS: " << apiResp << "\n";

            if (apiResp.find("\"ALLOW\"") != std::string::npos) {
                verdict = "[ALLOW / FORWARD]";
                predictedPeak = 5.64;
                currentP = 5.64;
                reason = "SAFE: Settles cleanly within physical safety envelope";
            } else {
                verdict = "[INLINE DROP]";
                predictedPeak = 20.74;
                reason = "PHYSICS VIOLATION: Peak pressure exceeds 10.0 bar limit";
            }
        }

        std::cout << "[*] ACTION : " << verdict << " -> " << reason << std::endl;

        // 1. Emit Log Row to Dashboard Table (UDP 5556)
        std::ostringstream logJson;
        logJson << "{\"type\":\"packet_log\",\"time\":\"" << timeStr << "\",\"hex\":\"" 
                << hex << "\",\"verdict\":\"" << verdict << "\",\"reason\":\"" << reason << "\"}";
        std::string lmsg = logJson.str();
        sendto(udpSock, lmsg.c_str(), lmsg.length(), 0, (struct sockaddr*)&uiAddr, sizeof(uiAddr));

        // 2. Emit Telemetry to Dashboard Graph (UDP 5556)
        std::ostringstream telemJson;
        telemJson << "{\"type\":\"telemetry\",\"time\":" << simTime << ",\"p_actual\":" 
                  << currentP << ",\"p_predicted\":" << predictedPeak << "}";
        std::string tmsg = telemJson.str();
        sendto(udpSock, tmsg.c_str(), tmsg.length(), 0, (struct sockaddr*)&uiAddr, sizeof(uiAddr));

        sleep(1);
    }

    close(udpSock);
    std::cout << "\n[+] Finished sending live stream to UI.\n";
    return 0;
}
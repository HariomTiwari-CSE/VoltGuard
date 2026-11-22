"""Passively observe Modbus/TCP packets and forward summaries to the dashboard."""

import argparse
import json
import socket
import sys
from datetime import datetime

try:
    from scapy.all import IP, IPv6, TCP, get_if_list, sniff
    from scapy.error import Scapy_Exception
except ImportError as error:
    print(
        "Scapy is required. Install Dashboardv2/requirements.txt first.",
        file=sys.stderr,
    )
    raise SystemExit(1) from error


DEFAULT_HOST = "127.0.0.1"
DEFAULT_UDP_PORT = 5556
MODBUS_TCP_PORT = 502
MAX_HEX_BYTES = 96

FUNCTION_NAMES = {
    0x01: "Read Coils",
    0x02: "Read Discrete Inputs",
    0x03: "Read Holding Registers",
    0x04: "Read Input Registers",
    0x05: "Write Single Coil",
    0x06: "Write Single Register",
    0x0F: "Write Multiple Coils",
    0x10: "Write Multiple Registers",
}


def classify_adu(payload):
    """Classify one packet payload without treating TCP segments as a stream."""
    if len(payload) < 7:
        return "PARTIAL", "Incomplete MBAP header; TCP stream is not reassembled."

    protocol_id = int.from_bytes(payload[2:4], "big")
    declared_length = int.from_bytes(payload[4:6], "big")

    if protocol_id != 0:
        return "REVIEW", f"Unexpected MBAP protocol ID {protocol_id}."

    if declared_length < 2 or declared_length > 254:
        return "REVIEW", f"Invalid MBAP length field: {declared_length}."

    expected_size = 6 + declared_length
    if len(payload) < expected_size:
        return "PARTIAL", (
            "Payload is shorter than its MBAP length; it may span TCP segments."
        )

    if len(payload) > expected_size:
        return "REVIEW", (
            "Payload contains extra bytes or multiple ADUs; "
            "TCP stream is not reassembled."
        )

    function_code = payload[7]
    function_name = FUNCTION_NAMES.get(
        function_code, f"Unknown function 0x{function_code:02X}"
    )
    return "COMPLETE", function_name


def packet_addresses(packet):
    if packet.haslayer(IP):
        return packet[IP].src, packet[IP].dst
    if packet.haslayer(IPv6):
        return packet[IPv6].src, packet[IPv6].dst
    return "unknown", "unknown"


def main():
    parser = argparse.ArgumentParser(
        description=(
            "Passively observe Modbus/TCP traffic and send summaries to "
            "the VoltGuard dashboard."
        )
    )
    parser.add_argument(
        "--interface",
        help="capture interface name (default: Scapy's default interface)",
    )
    parser.add_argument(
        "--list-interfaces",
        action="store_true",
        help="list capture interfaces and exit",
    )
    parser.add_argument("--udp-port", type=int, default=DEFAULT_UDP_PORT)
    args = parser.parse_args()

    if args.list_interfaces:
        print("\n".join(get_if_list()))
        return 0

    if not 1 <= args.udp_port <= 65535:
        parser.error("--udp-port must be between 1 and 65535")

    destination = (DEFAULT_HOST, args.udp_port)
    sender = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    packets_seen = 0

    def send_message(message):
        payload = json.dumps(message, separators=(",", ":")).encode("utf-8")
        sender.sendto(payload, destination)

    def process_packet(packet):
        nonlocal packets_seen

        if not packet.haslayer(TCP):
            return

        tcp = packet[TCP]
        if tcp.sport != MODBUS_TCP_PORT and tcp.dport != MODBUS_TCP_PORT:
            return

        payload = bytes(tcp.payload)
        if not payload:
            return

        source, destination_ip = packet_addresses(packet)
        frame_status, detail = classify_adu(payload)
        function = detail if frame_status == "COMPLETE" else "Unparsed segment"
        hex_payload = payload[:MAX_HEX_BYTES].hex(" ").upper()
        if len(payload) > MAX_HEX_BYTES:
            hex_payload += " ..."

        packets_seen += 1
        send_message(
            {
                "type": "packet_log",
                "time": datetime.now().astimezone().isoformat(timespec="seconds"),
                "source": f"{source}:{tcp.sport}",
                "function": function,
                "verdict": frame_status,
                "hex": hex_payload,
                "reason": detail,
            }
        )
        print(
            f"[PASSIVE] {source}:{tcp.sport} -> {destination_ip}:{tcp.dport} "
            f"{frame_status} {function}"
        )

    interface_name = args.interface or "default"
    print(
        f"Passively observing TCP port {MODBUS_TCP_PORT} "
        f"on {interface_name}; dashboard feed {destination[0]}:{destination[1]}."
    )
    print("Read-only monitor: it does not block or modify packets. Press Ctrl+C to stop.")

    try:
        while True:
            send_message(
                {
                    "type": "status",
                    "state": "listening",
                    "interface": interface_name,
                    "packets_seen": packets_seen,
                }
            )
            sniff(
                iface=args.interface,
                filter=f"tcp port {MODBUS_TCP_PORT}",
                timeout=1,
                store=False,
                prn=process_packet,
            )
    except KeyboardInterrupt:
        print(f"\nStopped. Observed {packets_seen} TCP/502 packets.")
    except (OSError, Scapy_Exception) as error:
        print(f"Capture or UDP forwarding failed: {error}", file=sys.stderr)
        return 1
    finally:
        sender.close()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

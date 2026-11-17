import os
import sys
import time
import json
import queue
import signal
import logging
import threading

from datetime import datetime

from scapy.all import sniff
from scapy.all import TCP
from scapy.all import IP

from parser_ips.modbus_parser import ModbusParser
from parser_ips.ips_engine import IPSEngine

APP_NAME = "VoltGuard"

VERSION = "1.0"

MODBUS_PORT = 502

LOG_PATH = "logs"

ALERT_PATH = "alerts"

RUNNING = True

os.makedirs(LOG_PATH, exist_ok=True)

os.makedirs(ALERT_PATH, exist_ok=True)

logging.basicConfig(
    filename=f"{LOG_PATH}/voltguard.log",
    level=logging.INFO,
    format="%(asctime)s - %(message)s"
)

print("=" * 60)
print("VOLTGUARD INDUSTRIAL IPS")
print("=" * 60)

class AlertManager:

    def __init__(self):

        self.alert_count = 0

    def raise_alert(self,
                    source_ip,
                    destination_ip,
                    reason):

        self.alert_count += 1

        alert_data = {
            "timestamp": str(datetime.now()),
            "source": source_ip,
            "destination": destination_ip,
            "reason": reason
        }

        filename = (
            f"{ALERT_PATH}/alert_"
            f"{self.alert_count}.json"
        )

        with open(filename, "w") as file:

            json.dump(
                alert_data,
                file,
                indent=4
            )

        logging.warning(
            f"ALERT: {reason}"
        )

        print()
        print("[ALERT GENERATED]")
        print(alert_data)
        print()

class Statistics:

    def __init__(self):

        self.total_packets = 0
        self.allowed_packets = 0
        self.blocked_packets = 0

    def increment_total(self):

        self.total_packets += 1

    def increment_allowed(self):

        self.allowed_packets += 1

    def increment_blocked(self):

        self.blocked_packets += 1

    def display(self):

        print()
        print("----------- STATISTICS -----------")
        print(
            f"Total   : {self.total_packets}"
        )
        print(
            f"Allowed : {self.allowed_packets}"
        )
        print(
            f"Blocked : {self.blocked_packets}"
        )
        print("----------------------------------")

parser = ModbusParser()

ips = IPSEngine()

alerts = AlertManager()

stats = Statistics()

packet_queue = queue.Queue()


def shutdown_handler(signum, frame):

    global RUNNING

    RUNNING = False

    print()
    print("[+] Stopping VoltGuard...")
    stats.display()

    sys.exit(0)


signal.signal(
    signal.SIGINT,
    shutdown_handler
)

signal.signal(
    signal.SIGTERM,
    shutdown_handler
)

def analyze_packet(packet):

    if not packet.haslayer(TCP):
        return

    tcp_layer = packet[TCP]

    if (
        tcp_layer.dport != MODBUS_PORT
        and
        tcp_layer.sport != MODBUS_PORT
    ):
        return

    payload = bytes(tcp_layer.payload)

    if len(payload) == 0:
        return

    stats.increment_total()

    frame = parser.parse_frame(payload)

    if frame is None:

        print(
            "[INVALID MODBUS FRAME]"
        )

        return

    result = ips.inspect(frame)

    source_ip = packet[IP].src
    destination_ip = packet[IP].dst

    if result["allowed"]:

        stats.increment_allowed()

        print(
            f"[ALLOW] {source_ip}"
        )

    else:

        stats.increment_blocked()

        print(
            f"[BLOCK] {source_ip}"
        )

        alerts.raise_alert(
            source_ip,
            destination_ip,
            result["reason"]
        )
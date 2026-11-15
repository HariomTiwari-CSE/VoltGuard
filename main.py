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

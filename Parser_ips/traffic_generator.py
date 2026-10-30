import socket
import json
import time
import random
import struct

UI_DEST = ("127.0.0.1", 5556)
API_DEST = ("127.0.0.1", 5555)
udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def build_modbus_hex(trans_id, rpm, valve):
    header = struct.pack(">HHHBBHHB", trans_id, 0x0000, 0x000B, 0x01, 0x10, 0x0001, 0x0002, 0x04)
    data = struct.pack(">HH", int(rpm) & 0xFFFF, int(valve) & 0xFFFF)
    return (header + data).hex().upper()

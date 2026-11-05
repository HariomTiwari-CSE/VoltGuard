import socket
import json
import time
import urllib.request

udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
ui_dest = ('127.0.0.1', 5556)

packets = [
    ("Normal Operator Command", "00010000000B0110000100020405DC0028", 1500, 40),
    ("Normal Ramping Setpoint", "00020000000B0110000100020407080032", 1800, 50),
    ("Malicious Water-Hammer Attack", "00030000000B011000010002040BB80002", 3000, 2),
    ("Catastrophic Overspeed Attack", "00040000000B01100001000204C3500050", 50000, 80)
]

print("[+] Starting Live Modbus Packet Stream to Dashboard...")

sim_time = 0.0
curr_p = 1.0

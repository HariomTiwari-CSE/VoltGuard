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

for name, hex_str, rpm, valve in packets:
    sim_time += 2.0
    now_str = time.strftime("%H:%M:%S")

    # Connect to Physics API (Port 5555)
    req = urllib.request.Request("http://127.0.0.1:5555", 
                                 data=json.dumps({"pump_rpm": rpm, "valve_opening": valve}).encode('utf-8'),
                                 headers={'Content-Type': 'application/json'})
    try:
        with urllib.request.urlopen(req, timeout=2) as resp:
            data = json.loads(resp.read().decode())
            verdict = "[ALLOW / FORWARD]" if data.get("verdict") == "ALLOW" else "[INLINE DROP]"
            reason = data.get("reason", "")
            peak_p = data.get("predicted_peak_bar", curr_p)
            if verdict == "[ALLOW / FORWARD]":
                curr_p = peak_p
    except Exception as e:
        # Direct raw TCP socket fallback for non-HTTP socket server
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect(('127.0.0.1', 5555))
        s.sendall(json.dumps({"pump_rpm": rpm, "valve_opening": valve}).encode() + b'\n')
        raw = s.recv(4096).decode()
        s.close()
        data = json.loads(raw)
        verdict = "[ALLOW / FORWARD]" if data.get("verdict") == "ALLOW" else "[INLINE DROP]"
        reason = data.get("reason", "")
        peak_p = data.get("predicted_peak_bar", curr_p)
        if verdict == "[ALLOW / FORWARD]":
            curr_p = peak_p

    print(f"[*] {now_str} | {hex_str} | {verdict} | {reason}")
 # 1. Send Row to UI Table
    log_payload = {
        "type": "packet_log",
        "time": now_str,
        "hex": hex_str,
        "verdict": verdict,
        "reason": reason
    }
    udp_sock.sendto(json.dumps(log_payload).encode(), ui_dest)

    # 2. Send Points to UI Graph
    telem_payload = {
        "type": "telemetry",
        "time": sim_time,
        "p_actual": curr_p,
        "p_predicted": peak_p
    }
    udp_sock.sendto(json.dumps(telem_payload).encode(), ui_dest)

    time.sleep(1.5)

print("[+] Stream completed.")

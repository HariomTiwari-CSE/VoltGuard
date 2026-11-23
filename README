# VoltGuard Passive Dashboard

The Qt dashboard displays packet observations from a separate Scapy capture
helper. It is a read-only monitor: it does not inject, modify, or block traffic.
TCP packet payloads are inspected individually; this helper does not reassemble
TCP streams, so split or coalesced Modbus/TCP ADUs may be marked partial or for
review.

## Requirements

- Qt 5 or Qt 6 Widgets and Network development packages
- CMake 3.16 or newer and a C++17 compiler
- Python 3.9 or newer
- Scapy (`python -m pip install -r Dashboardv2/requirements.txt`)
- On Windows, Npcap is required for packet capture. Capturing may require
  running the monitor terminal with elevated privileges.

## Build and run the dashboard (PowerShell)

From the repository root:

```powershell
cmake -S Dashboardv2 -B Dashboardv2/build
cmake --build Dashboardv2/build
.\Dashboardv2\build\VoltGuardDashboard.exe
```

## Start the passive monitor

In a second terminal, install Scapy and list available capture interfaces:

```powershell
python -m pip install -r Dashboardv2\requirements.txt
python Dashboardv2\live_modbus_monitor.py --list-interfaces
```

Start capture on the selected interface:

```powershell
python Dashboardv2\live_modbus_monitor.py --interface "YOUR INTERFACE NAME"
```

The monitor sends JSON messages to the dashboard over loopback UDP port 5556.
Start the dashboard first. If no interface is specified, Scapy selects its
default interface.

The dashboard only reports observed TCP port 502 packets and basic MBAP framing
status. “Complete” means the captured packet payload length matches the MBAP
length; it is not a security verdict or proof that a request is safe.

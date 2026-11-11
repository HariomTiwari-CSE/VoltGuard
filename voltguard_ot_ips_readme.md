# VoltGuard: Physical-Layer OT Intrusion Prevention System

VoltGuard is an inline, cyber-physical IPS designed for critical SCADA networks. It evaluates the physical consequences of Modbus/TCP commands in real time using an ordinary differential equation (ODE) digital twin, blocking equipment-damaging attacks (e.g., Stuxnet-style overspeed, water hammer) before commands reach field PLCs.

---

## Architecture Pipeline

```text
[ SCADA / Attacker ]
        │ (Modbus/TCP)
        ▼
[ Parser & Traffic Engine ] (Parser_ips/) ──TCP:5555──▶ [ ODE Physics Core ] (Physics_core/)
        │                                                    │ (ALLOW / DROP)
        │                                                    ▼
        └──────────────────── UDP:5556 ─────────────────▶ [ Qt6 Telemetry GUI ] (Dashboard_ui/)
```

---

## Directory Structure

```text
Voltgaurd/
├── run_voltguard.sh             # Single-command launcher (startup & cleanup)
├── Physics_core/
│   └── Simulation_api.py        # ODE hydraulic transient solver (Port 5555)
├── Parser_ips/
│   ├── modbus_parser.cpp/.hpp   # Modbus/TCP protocol frame decoder
│   ├── ips_engine.cpp           # Inline filter & UDP telemetry broadcaster
│   └── traffic_generator.py     # SCADA & attack traffic stream generator
└── Dashboard_ui/
    ├── CMakeLists.txt           # Qt6 build definition
    ├── mainwindow.cpp/.h        # Real-time telemetry receiver (UDP 5556)
    ├── physics_plot_widget.cpp  # Smooth 100ms hydraulic waveform widget
    └── traffic_log_model.cpp    # Color-coded decision table (ALLOW / DROP)
```

---

## Key Safety Constraints

* **Maximum Operating Pressure ($P_{\max}$):** $10.0\text{ Bar}$ (Commands exceeding this are dropped inline).
* **Impeller Speed Ceiling ($\omega_{\max}$):** $3600\text{ RPM}$ (Blocks destructive overspeed attacks).
* **Water-Hammer Protection:** Drops rapid valve closures ($\le 3\%$) occurring at pump speeds $> 2200\text{ RPM}$.
* **Evaluation Latency:** Sub-$10\text{ ms}$ inline decision-making.

---

## Quick Start

### 1. Build & Dependencies

```bash
# Install dependencies (Debian/Ubuntu)
sudo apt install build-essential cmake qt6-base-dev libqt6charts6-dev python3-venv

# Set up Python virtual environment
python3 -m venv venv && source venv/bin/activate
pip install numpy scipy

# Build Qt6 Dashboard
cd Dashboard_ui && mkdir -p build && cd build
cmake .. && cmake --build . && cd ../..
```

### 2. Run Entire Stack

Launch all services with a single script:

```bash
./run_voltguard.sh
```

*(Press `Ctrl + C` in the terminal to cleanly terminate all background services.)*

---

## Port Allocation

| Port | Protocol | Usage |
| :--- | :--- | :--- |
| **5555** | TCP | Synchronous digital twin physics evaluation |
| **5556** | UDP | Telemetry waveform and packet audit streaming to GUI |
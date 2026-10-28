#!/bin/bash

BASE_DIR="/home/dragon/Desktop/Infotact/Voltgaurd"
cd "$BASE_DIR" || exit 1

echo "======================================================"
echo "      STARTING VOLTGUARD INDUSTRIAL OT FIREWALL      "
echo "======================================================"

# Kill any leftover processes on ports 5555 and 5556
pkill -9 -f VoltGuardUI 2>/dev/null
pkill -9 -f Simulation_api.py 2>/dev/null
pkill -9 -f traffic_generator.py 2>/dev/null
sleep 1


# Physics API Server Start (Port 5555)
echo "[1/3] Launching Physics Simulation API..."
source venv/bin/activate
python Physics_core/Simulation_api.py > /dev/null 2>&1 &
PID_PHYSICS=$!
sleep 1.5


# Qt6 Modular Dashboard Start (Port 5556)
echo "[2/3] Launching VoltGuard Qt6 Dashboard..."
./Dashboard_ui/build/VoltGuardUI &
PID_UI=$!
sleep 2
# Clean-up function on Ctrl+C
cleanup() {
    echo ""
    echo "[!] Shutting down VoltGuard stack..."
    kill -9 "$PID_GEN" 2>/dev/null
    kill -9 "$PID_UI" 2>/dev/null
    kill -9 "$PID_PHYSICS" 2>/dev/null
    pkill -9 -f "Physics_core/Simulation_api.py" 2>/dev/null
    pkill -9 -f "VoltGuardUI" 2>/dev/null
    pkill -9 -f "traffic_generator.py" 2>/dev/null
    echo "[+] All VoltGuard services cleanly terminated."
    exit 0
}

trap cleanup SIGINT SIGTERM

# Traffic & Attack Generator Start
echo "[3/3] Launching Autonomous Modbus Traffic Stream..."
echo "------------------------------------------------------"
echo "Press [Ctrl+C] to stop all services simultaneously."
echo "------------------------------------------------------"

# Path check for traffic_generator.py
if [ -f "$BASE_DIR/Parser_ips/traffic_generator.py" ]; then
    python "$BASE_DIR/Parser_ips/traffic_generator.py"
elif [ -f "$BASE_DIR/traffic_generator.py" ]; then
    python "$BASE_DIR/traffic_generator.py"
else
    echo "[-] Error: traffic_generator.py not found!"
fi

#!/bin/bash

BASE_DIR="/home/dragon/Desktop/Infotact/Voltgaurd"
cd "$BASE_DIR" || exit 1

echo "======================================================"
echo "      STARTING VOLTGUARD INDUSTRIAL OT FIREWALL      "
echo "======================================================"

# 0. Kill any leftover processes on ports 5555 and 5556
pkill -9 -f VoltGuardUI 2>/dev/null
pkill -9 -f Simulation_api.py 2>/dev/null
pkill -9 -f traffic_generator.py 2>/dev/null
sleep 1

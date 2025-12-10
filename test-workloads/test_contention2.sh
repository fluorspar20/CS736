#!/bin/bash

# Clean up
pkill mem_noise

echo "============================================================"
echo "TEST 1: High Priority (Sticky CPU) vs Memory Pressure"
echo "Class 1 (200ms) resists switching, preserving its cache."
echo "============================================================"

# 1. Start Memory Noise as Class 1
./mem_noise &
NOISE_PID=$!
sleep 0.5 # Give it time to allocate memory
echo "1" > /tidinfo/$NOISE_PID.txt
echo $NOISE_PID | sudo tee /sys/fs/ghost/enclave_1/tasks > /dev/null

# 2. Run Fib as Class 1
python3 run_alps_test.py 42 1

# Clean up
kill $NOISE_PID
rm /tidinfo/$NOISE_PID.txt
sleep 2

echo ""
echo "============================================================"
echo "TEST 2: Low Priority (Thrashing) vs Memory Pressure"
echo "Class 0 (8ms) yields frequently. Cache is flushed constantly."
echo "============================================================"

# 1. Start Memory Noise as Class 0
./mem_noise &
NOISE_PID=$!
sleep 0.5
echo "0" > /tidinfo/$NOISE_PID.txt
echo $NOISE_PID | sudo tee /sys/fs/ghost/enclave_1/tasks > /dev/null

# 2. Run Fib as Class 0
python3 run_alps_test.py 42 0

# Cleanup
kill $NOISE_PID
rm /tidinfo/$NOISE_PID.txt

#!/bin/bash

# Clean up
pkill mem_noise

echo "============================================================"
echo "TEST 1: ALPS Mode (Class 1 - 200ms Slice)"
echo "The task holds the CPU, keeping its 16MB buffer in L3 Cache."
echo "============================================================"

# 1. Start Cache Destroyer as Class 1 (Equal Competitor)
./mem_noise &
NOISE_PID=$!
sleep 0.5
echo "1" > /tidinfo/$NOISE_PID.txt
echo $NOISE_PID | sudo tee /sys/fs/ghost/enclave_1/tasks > /dev/null

# 2. Run Fragile Task as Class 1
# Note: We need to update run_alps_test.py to use ./fragile_task
python3 run_alps_test.py 0 1 --custom_cmd "./fragile_task"

# Clean up
kill $NOISE_PID
rm /tidinfo/$NOISE_PID.txt
sleep 2

echo ""
echo "============================================================"
echo "TEST 2: CFS Mode (Class 0 - 8ms Slice)"
echo "The task yields constantly. Cache is wiped by Noise every 8ms."
echo "============================================================"

# 1. Start Cache Destroyer as Class 0
./mem_noise &
NOISE_PID=$!
sleep 0.5
echo "0" > /tidinfo/$NOISE_PID.txt
echo $NOISE_PID | sudo tee /sys/fs/ghost/enclave_1/tasks > /dev/null

# 2. Run Fragile Task as Class 0
python3 run_alps_test.py 0 0 --custom_cmd "./fragile_task"

# Cleanup
kill $NOISE_PID
rm /tidinfo/$NOISE_PID.txt

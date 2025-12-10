#!/bin/bash

# Clean up
pkill mem_noise

echo "============================================================"
echo "TEST 3: CFS Baseline (Pinned to CPU 0)"
echo "Forces Linux CFS to handle contention on a single core."
echo "============================================================"

# 1. Start Noise on CPU 0
# taskset -c 0 ensures it stays on the first core
taskset -c 0 ./mem_noise &
NOISE_PID=$!
sleep 0.5

# 2. Run Fragile Task on CPU 0
# We use taskset here too. 
# We don't use 'ghost_run' or /tidinfo/ because we want standard Linux behavior.
taskset -c 0 ./fragile_task

# Cleanup
kill $NOISE_PID

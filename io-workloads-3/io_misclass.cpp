#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include <cmath>
#include <unistd.h>
#include <cstdlib>

// Configuration
const int BURST_MS = 1; // Tiny burst (Real behavior = Short)

void burn_cpu(int ms) {
    auto start = std::chrono::high_resolution_clock::now();
    while (true) {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - start).count() >= ms) break;
        volatile int x = 1 + 1;
    }
}

void setup_alps(int class_id) {
    pid_t pid = getpid();
    // 1. Write Policy
    std::ofstream info_file("/tidinfo/" + std::to_string(pid) + ".txt");
    // FORCE MISCLASSIFICATION: Even if input is Class 1, we write Class 5 (Low Prio)
    // asking the scheduler to treat us as a long-running batch job.
    if (info_file.is_open()) { info_file << "5"; info_file.close(); }
    
    // 2. Move to ghOSt
    std::ofstream tasks_file("/sys/fs/ghost/enclave_1/tasks");
    if (tasks_file.is_open()) { tasks_file << pid; tasks_file.close(); }
}

int main(int argc, char* argv[]) {
    if (argc < 4) return 1;
    int id = std::atoi(argv[1]);
    int n = std::atoi(argv[2]);

    // We ignore the 'real' class and force the setup to Class 5 inside
    setup_alps(5); 
    
    // PHASE 1: Tiny Burst
    burn_cpu(BURST_MS);

    // PHASE 2: Sleep (The I/O Wait)
    int sleep_ms = 100; 
    auto start_sleep = std::chrono::high_resolution_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    auto end_sleep = std::chrono::high_resolution_clock::now();

    // PHASE 3: Wakeup & Critical Section
    // If ALPS is broken, we will stall here behind Class 3 tasks.
    burn_cpu(BURST_MS); 

    // METRICS
    long actual_sleep = std::chrono::duration_cast<std::chrono::milliseconds>(end_sleep - start_sleep).count();
    long latency = actual_sleep - sleep_ms;
    if (latency < 0) latency = 0;

    std::cout << "[Result] ID:" << id << " Type:IO_Misclass Latency:" << latency << "ms" << std::endl;
    return 0;
}

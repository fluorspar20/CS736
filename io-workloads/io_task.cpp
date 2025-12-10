#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include <cmath>
#include <unistd.h> // for getpid
#include <cstdlib>

// --- CONFIGURATION ---
const int BURST_MS = 5; // The "Short CPU Burst" (5ms)
// ---------------------

// Helper to burn CPU cycles (Simulating request processing)
void burn_cpu(int ms) {
    auto start = std::chrono::high_resolution_clock::now();
    while (true) {
        auto now = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() >= ms)
            break;
        volatile int x = 1 + 1; // Prevent optimization
    }
}

// Helper to mimic the "Heavy Tail" sleep durations
int calculate_sleep_duration(int n) {
    // Input 30 -> ~290ms
    // Input 45 -> ~125,000ms
    double duration = std::pow(1.6, (n - 25)) * 10; 
    return (int)duration;
}

void setup_alps(int class_id) {
    pid_t pid = getpid();
    
    // 1. Write Policy
    std::string info_path = "/tidinfo/" + std::to_string(pid) + ".txt";
    std::ofstream info_file(info_path);
    if (info_file.is_open()) {
        info_file << class_id;
        info_file.close();
    }

    // 2. Move to ghOSt Enclave
    std::ofstream tasks_file("/sys/fs/ghost/enclave_1/tasks");
    if (tasks_file.is_open()) {
        tasks_file << pid;
        tasks_file.close();
    }
}

int main(int argc, char* argv[]) {
    if (argc < 4) return 1;

    int id = std::atoi(argv[1]);
    int n = std::atoi(argv[2]);
    int class_id = std::atoi(argv[3]);

    // --- ALPS SETUP ---
    setup_alps(class_id);
    
    // Warmup
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // --- PHASE 1: The "Short CPU Burst" ---
    // This makes the scheduler think we are a running task
    auto start_total = std::chrono::high_resolution_clock::now();
    burn_cpu(BURST_MS);

    // --- PHASE 2: The "Long Wait" (I/O) ---
    // We yield the CPU here. 
    int sleep_ms = calculate_sleep_duration(n);
    auto start_sleep = std::chrono::high_resolution_clock::now();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    
    auto end_sleep = std::chrono::high_resolution_clock::now();

    // --- PHASE 3: Post-Processing ---
    // Tiny work after waking up
    burn_cpu(1); 

    auto end_total = std::chrono::high_resolution_clock::now();

    // METRICS
    // We want to know the "Wakeup Latency": 
    // Actual Sleep Time - Requested Sleep Time
    long actual_sleep = std::chrono::duration_cast<std::chrono::milliseconds>(end_sleep - start_sleep).count();
    long latency = actual_sleep - sleep_ms;
    if (latency < 0) latency = 0;

    long total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_total - start_total).count();

    std::cout << "[Result] ID:" << id 
              << " Class:" << class_id 
              << " TargetSleep:" << sleep_ms << "ms"
              << " Latency:" << latency << "ms"
              << " Total:" << total_time << "ms" << std::endl;

    return 0;
}

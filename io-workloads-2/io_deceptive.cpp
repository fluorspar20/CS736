#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include <cmath>
#include <unistd.h>
#include <cstdlib>

// Configuration: 5ms Burst is enough to trigger "Active" status but small enough to be Class 1
const int BURST_MS = 50; 

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
    std::ofstream info_file("/tidinfo/" + std::to_string(pid) + ".txt");
    if (info_file.is_open()) { info_file << class_id; info_file.close(); }
    std::ofstream tasks_file("/sys/fs/ghost/enclave_1/tasks");
    if (tasks_file.is_open()) { tasks_file << pid; tasks_file.close(); }
}

int main(int argc, char* argv[]) {
    if (argc < 4) return 1;
    int id = std::atoi(argv[1]);
    int n = std::atoi(argv[2]);
    int class_id = std::atoi(argv[3]);

    setup_alps(class_id);
    
    // 1. The "Deception": Short CPU Burst
    burn_cpu(BURST_MS);

    // 2. The "Reality": Long I/O Wait
    // Map input 'n' to sleep time (30 -> ~100ms)
    int sleep_ms = (int)(std::pow(1.5, (n - 25)) * 10); 
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    
    // 3. Wakeup
    burn_cpu(1); 

    // Only print if you want to debug I/O latencies, otherwise silence to focus on CPU results
    std::cout << "[Result] ID:" << id << " Type:IO Class:" << class_id << " Sleep:" << sleep_ms << "ms" << std::endl;
    return 0;
}

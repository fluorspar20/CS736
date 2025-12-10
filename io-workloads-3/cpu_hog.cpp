#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <unistd.h>
#include <cstdlib>
#include <cmath>

// Burn CPU based on trace parameter 'n'
void burn_fib_simulation(int n) {
    // Approx duration mapping for 'fib'
    long duration_ms = (long)(std::pow(1.6, (n - 25)) * 2); 
    auto start = std::chrono::high_resolution_clock::now();
    while (true) {
        auto now = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() >= duration_ms) break;
        volatile int x = 1 + 1;
    }
}

void setup_alps(int class_id) {
    pid_t pid = getpid();
    std::ofstream info_file("/tidinfo/" + std::to_string(pid) + ".txt");
    // This is Class 3 (Medium Priority)
    if (info_file.is_open()) { info_file << "3"; info_file.close(); }
    
    std::ofstream tasks_file("/sys/fs/ghost/enclave_1/tasks");
    if (tasks_file.is_open()) { tasks_file << pid; tasks_file.close(); }
}

int main(int argc, char* argv[]) {
    if (argc < 4) return 1;
    int n = std::atoi(argv[2]);
    setup_alps(3); // Register as Class 3
    
    burn_fib_simulation(n);
    return 0;
}

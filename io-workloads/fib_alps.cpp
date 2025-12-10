#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <unistd.h> // for getpid, usleep
#include <cstdlib>

long long fib(int n) {
    if (n <= 1) return n;
    return fib(n-1) + fib(n-2);
}

void setup_alps(int class_id) {
    pid_t pid = getpid();
    
    // 1. Write Policy to /tidinfo/ (The Side Channel)
    // This tells the C++ Agent which Class (Time Slice) to use
    std::string info_path = "/tidinfo/" + std::to_string(pid) + ".txt";
    std::ofstream info_file(info_path);
    if (info_file.is_open()) {
        info_file << class_id;
        info_file.close();
    } else {
        std::cerr << "[Error] Could not write to " << info_path << std::endl;
    }

    // 2. Move to ghOSt Enclave (Self-Migration)
    std::ofstream tasks_file("/sys/fs/ghost/enclave_1/tasks");
    if (tasks_file.is_open()) {
        tasks_file << pid;
        tasks_file.close();
    } else {
        // If this fails, we run on CFS (Standard Linux)
        // std::cerr << "[Warning] Could not move to ghOSt. Agent running?" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // start.c passes: {filename, id, para, class, NULL}
    // argv[0]: filename
    // argv[1]: ID
    // argv[2]: Parameter (Fib Number)
    // argv[3]: Function Class
    
    if (argc < 4) {
        std::cerr << "Usage: ./fib_alps <id> <n> <class>" << std::endl;
        return 1;
    }

    int id = std::atoi(argv[1]);
    int n = std::atoi(argv[2]);
    int class_id = std::atoi(argv[3]);

    // --- ALPS SETUP ---
    setup_alps(class_id);
    
    // Give the agent a tiny moment to pick us up (Optional, but helps stability)
    usleep(1000); 

    // --- WORKLOAD ---
    auto start = std::chrono::high_resolution_clock::now();
    long long result = fib(n);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Output formatted for easy parsing
    std::cout << "[Result] ID:" << id 
              << " Class:" << class_id 
              << " Fib(" << n << ")"
              << " Time:" << duration << "ms" << std::endl;

    return 0;
}

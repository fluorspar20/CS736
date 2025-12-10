#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <unistd.h>
#include <cstdlib>

long long fib(int n) {
    if (n <= 1) return n;
    return fib(n-1) + fib(n-2);
}

void setup_alps(int class_id) {
    pid_t pid = getpid();
    // Write Policy
    std::ofstream info_file("/tidinfo/" + std::to_string(pid) + ".txt");
    if (info_file.is_open()) { info_file << class_id; info_file.close(); }
    // Move to ghOSt
    std::ofstream tasks_file("/sys/fs/ghost/enclave_1/tasks");
    if (tasks_file.is_open()) { tasks_file << pid; tasks_file.close(); }
}

int main(int argc, char* argv[]) {
    if (argc < 4) return 1;
    int id = std::atoi(argv[1]);
    int n = std::atoi(argv[2]);
    int class_id = std::atoi(argv[3]);

    setup_alps(class_id);
    
    auto start = std::chrono::high_resolution_clock::now();
    long long result = fib(n);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "[Result] ID:" << id 
              << " Type:CPU"
              << " Class:" << class_id 
              << " Time:" << duration << "ms" << std::endl;
    return 0;
}

#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <random>
#include <chrono>
#include <unistd.h> // for getpid

// 16MB Buffer (Fits in L3, but fragile)
const size_t ARRAY_SIZE = 16 * 1024 * 1024 / sizeof(int); 
const int ITERATIONS = 50000000; // 50 Million accesses

int main() {
    // 1. Setup: Create a random pointer chain
    // (index i contains the index of the next hop)
    std::vector<int> memory(ARRAY_SIZE);
    std::iota(memory.begin(), memory.end(), 0);
    
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(memory.begin(), memory.end(), g);

    // 2. Print PID for the ALPS Manager
    std::cout << "PID:" << getpid() << std::endl;
    
    // Wait for ALPS setup
    usleep(20000); 

    // 3. The Workload: Pointer Chasing
    // This is extremely sensitive to cache eviction.
    int next_index = 0;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < ITERATIONS; ++i) {
        // Jump to the next random location
        // If cache is cold, this stalls the CPU for ~100ns
        next_index = memory[next_index];
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Prevent compiler optimization
    if (next_index == -12345) std::cout << "Impossible"; 

    std::cout << "[Result] PointerChase Time: " << duration << " ms" << std::endl;
    return 0;
}

#include <iostream>
#include <vector>
#include <unistd.h>

int main() {
    // Allocate 100MB (Larger than most L3 caches)
    const size_t SIZE = 100 * 1024 * 1024; 
    std::vector<int> data(SIZE / sizeof(int), 1);
    
    std::cout << "Memory Noise PID: " << getpid() << std::endl;

    // Constantly trashing the cache
    while (true) {
        for (size_t i = 0; i < data.size(); i += 1024) { // Stride to hit different cache lines
            data[i]++;
        }
    }
    return 0;
}

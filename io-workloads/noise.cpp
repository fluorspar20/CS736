#include <iostream>
#include <unistd.h> // for getpid

int main() {
    std::cout << "CPU Noise PID: " << getpid() << std::endl;
    
    // Infinite loop to burn 100% CPU
    // 'volatile' prevents the compiler from optimizing this away
    while (true) {
        volatile int x = 1 + 1;
    }
    return 0;
}

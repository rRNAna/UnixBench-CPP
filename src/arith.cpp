/**
 * @file        arith.cpp
 * @brief       C++ refactored UnixBench arith test module
 * @author      rRNA
 * @version     3.5.0
 * @date        04-28-2025
 *
 * @details
 * This file is a C++ reconstruction of the original C file.
 * Original project address: https://github.com/kdlucas/byte-unixbench/tree/v5.1.3
 *
 * This module is used to perform basic arithmetic performance tests on integer
 * and floating point numbers.
 */
#include <iostream>
#include <cstdlib>
#include <csignal>
#include <unistd.h> // For sleep function

// Global variable to store iterations
volatile unsigned long iter = 0;

// This function is called when the alarm expires
void report(int signum) {
    std::cerr << "COUNT|" << iter << "|1|lps" << std::endl;
    exit(0);
}

// Function to do some dumb stuff
int dumb_stuff(int i) {
    #ifndef arithoh
    int x = 0, y = 0, z = 0;
    for (int i = 2; i <= 101; ++i) {
        x = i;
        y = x * x;
        z += y / (y - 1);
    }
    return x + y + z;
    #else
    return 0;
    #endif
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " duration" << std::endl;
        return 1;
    }

    int duration = std::atoi(argv[1]);

    // Set up alarm call
    std::signal(SIGALRM, report);
    alarm(duration);

    int result = 0;
    // This loop will be interrupted by the alarm call
    while (true) {
        ++iter;
        result = dumb_stuff(result);
    }

    return 0;
}

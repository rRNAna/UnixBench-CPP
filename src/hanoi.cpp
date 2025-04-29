/**
 * @file        hanoi.cpp
 * @brief       C++ refactored UnixBench arith test module
 * @author      rRNA
 * @version     3.4.0
 * @date        04-28-2025
 *
 * @details
 * This file is a C++ rewrite of hanoi.c from the original UnixBench project.
 * Original project address: https://github.com/kdlucas/byte-unixbench/tree/v5.1.3
 *
 *
 *
 */

#include <iostream>
#include <cstdlib>
#include "timeit.cpp"  // The wake_me function is defined in timeit.cpp
#include <csignal>

using namespace std;

// Inline function replaces macro definition other(i,j)
inline int other(int i, int j) {
    return 6 - (i + j);
}

// Global variables
unsigned long iter = 0;
int num[4] = {0};  // Initialized to 0
long cnt = 0;

void report(int sig) {
    std::cerr << "COUNT|" << iter << "|1|lps" << std::endl;
    exit(0);
}

void mov(int n, int f, int t) {
    if(n == 1) {
        num[f]--;
        num[t]++;
        return;
    }
    int o = other(f, t);
    mov(n - 1, f, o);
    mov(1, f, t);
    mov(n - 1, o, t);
}

int main(int argc, char* argv[]) {
    int disk = 10;  // The default number of disks is 10
    int duration;

    if(argc < 2) {
        cerr << "Usage: " << argv[0] << " duration [disks]" << endl;
        exit(1);
    }

    duration = atoi(argv[1]);
    if(argc > 2) {
        disk = atoi(argv[2]);
    }
    num[1] = disk;

    // Set a timer and call the report function when the duration is reached
    wake_me(duration, report);

    while(true) {
        mov(disk, 1, 3);
        iter++;
    }

    return 0;
}
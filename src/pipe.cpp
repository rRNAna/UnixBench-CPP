/**
 * @file        pipe.cpp
 * @brief       C++ refactored UnixBench arith test module
 * @author      rRNA
 * @version     3.4.0
 * @date        04-28-2025
 *
 * @details
 * This file is a C++ rewrite of pipe.c from the original UnixBench project.
 * Original project address: https://github.com/kdlucas/byte-unixbench/tree/v5.1.3
 *
 *
 *
 */

#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include "timeit.cpp"  // The wake_me function is defined in timeit.cpp

using namespace std;

unsigned long iter;

void report(int sig) {
    std::cerr << "COUNT|" << iter << "|1|lps" << std::endl;
    exit(0);
}

int main(int argc, char* argv[]) {
    char buf[512];
    int pvec[2];
    int duration;

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " duration" << std::endl;
        exit(1);
    }

    duration = atoi(argv[1]);

    // Create a pipeline. If the creation fails, output an error message and exit.
    if (pipe(pvec) == -1) {
        std::cerr << "pipe creation failed, error " << errno << std::endl;
        exit(1);
    }

    // Call wake_me to set a timer, and call the report function after duration seconds have passed.
    wake_me(duration, report);
    iter = 0;

    // The pipeline data is continuously written and read in a loop, and the counter iter is continuously increased.
    while (true) {
        if (write(pvec[1], buf, sizeof(buf)) != sizeof(buf)) {
            if (errno != EINTR && errno != 0)
                std::cerr << "write failed, error " << errno << std::endl;
        }
        if (read(pvec[0], buf, sizeof(buf)) != sizeof(buf)) {
            if (errno != EINTR && errno != 0)
                std::cerr << "read failed, error " << errno << std::endl;
        }
        iter++;
    }

    return 0;
}
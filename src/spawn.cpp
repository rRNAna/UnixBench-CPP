/**
 * @file        spawn.cpp
 * @brief       C++ refactored UnixBench arith test module
 * @author      rRNA
 * @version     3.4.0
 * @date        04-28-2025
 *
 * @details
 * This file is a C++ rewrite of spawn.c from the original UnixBench project.
 * Original project address: https://github.com/kdlucas/byte-unixbench/tree/v5.1.3
 *
 *
 *
 */

#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <cstdio>
#include "timeit.cpp" // The wake_me function is defined in timeit.cpp

#include <sys/wait.h>
using namespace std;

// Global iteration count variable
volatile unsigned long iter = 0;

// The callback function called when the timer is triggered
void report(int sig) {
    char buf[64];
    int len = snprintf(buf, sizeof(buf), "COUNT|%lu|1|lps\n", iter);
    write(STDERR_FILENO, buf, len);
    _exit(0);  // Avoiding C++ exit cleanup
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " duration" << std::endl;
        exit(1);
    }

    int duration = std::atoi(argv[1]);
    if (duration < 1) {
        std::cerr << "Invalid duration: " << argv[1] << std::endl;
        exit(1);
    }

    wake_me(duration, report);
    iter = 0;

    int status = 0;
    while (true) {
        pid_t pid = fork();
        if (pid == 0) {
            exit(0); // child
        } else if (pid < 0) {
            std::cerr << "Fork failed at iteration " << iter << std::endl;
            perror("Reason");
            exit(2);
        } else {
            wait(&status);
            if (status != 0) {
                std::cerr << "Bad wait status: 0x" << std::hex << status << std::endl;
                exit(2);
            }
            iter++;
        }
    }
}

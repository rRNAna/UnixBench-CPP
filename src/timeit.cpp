/**
 * @file        timeit.cpp
 * @brief       C++ refactored UnixBench arith test module
 * @author      rRNA
 * @version     3.4.0
 * @date        04-28-2025
 *
 * @details
 * This file is a C++ rewrite of timeit.c from the original UnixBench project.
 * Original project address: https://github.com/kdlucas/byte-unixbench/tree/v5.1.3
 *
 *
 *
 */

#include <csignal>
#include <unistd.h>

void wake_me(int seconds, void (*func)(int)) {
    // Set the signal handler, the callback function must now accept an int parameter
    std::signal(SIGALRM, func);
    // Start Timer
    alarm(seconds);
}
/**
 * @file        execl.cpp
 * @brief       C++ refactored UnixBench arith test module
 * @author      rRNA
 * @version     3.5.0
 * @date        04-29-2025
 *
 * @details
 * This file is a C++ rewrite of execl.c from the original UnixBench project.
 * Original project address: https://github.com/kdlucas/byte-unixbench/tree/v5.1.3
 *
 *
 *
 */

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <ctime>

char bss[8 * 1024];

#define main dummy
#include "big.cpp"
#undef main

int main(int argc, char* argv[]) {
    unsigned long iter = 0;
    char *ptr;
    char *fullpath;
    int duration;
    char count_str[32], start_str[32], path_str[256], *dur_str;
    time_t start_time, this_time;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s duration\n", argv[0]);
        exit(1);
    }

    duration = atoi(argv[1]);
    if (duration > 0) {
        dur_str = argv[1];
        ptr = getenv("UB_BINDIR");
        if (ptr != nullptr) {
            snprintf(path_str, sizeof(path_str), "%s/execl", ptr);
            fullpath = path_str;
        } else {
            fullpath = argv[0];
        }
        time(&start_time);
    } else {
        if (argc < 5) {
            fprintf(stderr, "Not enough arguments for execl'ed invocation\n");
            exit(1);
        }
        duration = atoi(argv[2]);
        dur_str = argv[2];
        iter = (unsigned long)atoi(argv[3]);
        sscanf(argv[4], "%lu", (unsigned long *)&start_time);
        fullpath = argv[0];
    }

    snprintf(count_str, sizeof(count_str), "%lu", ++iter);
    snprintf(start_str, sizeof(start_str), "%lu", (unsigned long)start_time);
    time(&this_time);

    if (this_time - start_time >= duration) {
        fprintf(stderr, "COUNT|%lu|1|lps\n", iter);
        exit(0);
    }

    execl(fullpath, fullpath, "0", dur_str, count_str, start_str, (char*)0);

    fprintf(stderr, "Exec failed at iteration %lu\n", iter);
    perror("Reason");
    exit(1);
}

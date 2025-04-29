/**
 * @file        looper.cpp
 * @brief       C++ refactored UnixBench arith test module
 * @author      rRNA
 * @version     1.5.0
 * @date        04-28-2025
 *
 * @details
 * This file is a C++ rewrite of looper.c from the original UnixBench project.
 * Original project address: https://github.com/kdlucas/byte-unixbench/tree/v5.1.3
 *
 *
 *
 */

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <csignal>
#include "timeit.cpp"  // The wake_me function is defined in timeit.cpp

using namespace std;

// Global variables
unsigned long iter = 0;
char* cmd_argv[28]; // Save commands and parameters
int  cmd_argc = 0;

// Timer signal processing function, when the duration is reached,
// call report() to output the number of iterations and then exit
void report(int sig) {
    // Make sure the output goes to stdout so Run.py can capture it
    cout << "COUNT|" << iter << "|60|lpm" << endl;
    exit(0);
}

int main(int argc, char* argv[]) {
    int slave, count, duration;
    int status;

    // Parameter check: requires at least two parameters: duration and command
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " duration command [args..]" << endl;
        cerr << "  duration in seconds" << endl;
        exit(1);
    }

    // Parsing duration parameters
    duration = atoi(argv[1]);
    if (duration < 1) {
        cerr << "Usage: " << argv[0] << " duration command [args..]" << endl;
        cerr << "  duration in seconds" << endl;
        exit(1);
    }

    // Parsing commands and their arguments
    cmd_argc = argc - 2;
    for (count = 2; count < argc; ++count) {
        cmd_argv[count - 2] = argv[count];
    }
    // Note: The cmd_argv array must end with NULL, as required by execvp
    cmd_argv[cmd_argc] = nullptr;

#ifdef DEBUG
    cout << "Command: <<" << cmd_argv[0] << ">>";
    for(count = 1; count < cmd_argc; ++count)
         cout << " <" << cmd_argv[count] << ">";
    cout << endl;
    exit(0);
#endif

    iter = 0;
    // Set a timer and call the report() function after duration seconds
    wake_me(duration, report);

    // The main loop continuously forks to create child processes to
    // execute the specified commands
    while (true) {
        slave = fork();
        if (slave == 0) {
            // Subprocess: Execute command
            execvp(cmd_argv[0], cmd_argv);
            // execvp returns an exit code of 99 when execution fails.
            exit(99);
        } else if (slave < 0) {
            // Fork failed
            cerr << "Fork failed at iteration " << iter << endl;
            perror("Reason");
            exit(2);
        } else {
            // Parent process: Wait for the child process to end
            wait(&status);
            // Determine whether the child process exit status indicates execvp failure
            if (status == (99 << 8)) {
                cerr << "Command \"" << cmd_argv[0] << "\" didn't exec" << endl;
                exit(2);
            } else if (status != 0) {
                cerr << "Bad wait status: 0x" << std::hex << status << endl;
                exit(2);
            }
        }
        iter++;
    }

    return 0;
}

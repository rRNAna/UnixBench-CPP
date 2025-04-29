/**
 * @file        syscall.cpp
 * @brief       C++ refactored UnixBench arith test module
 * @author      rRNA
 * @version     3.5.0
 * @date        04-29-2025
 *
 * @details
 * This file is a C++ rewrite of syscall.c from the original UnixBench project.
 * Original project address: https://github.com/kdlucas/byte-unixbench/tree/v5.1.3
 *
 *
 *
 */

#include <iostream>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>

// The timing tool implementation has been moved to timeit.cpp
// to keep it consistent with the original version
#include "timeit.cpp"

static unsigned long iter = 0;

// The report function needs to match the signature of wake_me:
// it receives the remaining seconds parameter
void report(int /*unused*/) {
    std::cerr << "COUNT|" << iter << "|1|lps" << std::endl;
    std::exit(0);
}

int create_fd() {
    int fd[2];
    if (pipe(fd) != 0 || close(fd[1]) != 0) {
        std::exit(1);
    }
    return fd[0];
}

int main(int argc, char* argv[]) {
    std::string test;
    int duration;
    int fd;

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " duration [ test ]" << std::endl
                  << "test is one of:" << std::endl
                  << "  \"mix\" (default), \"close\", \"getpid\", \"exec\"" << std::endl;
        return EXIT_FAILURE;
    }

    test = (argc > 2) ? argv[2] : "mix";
    duration = std::atoi(argv[1]);

    iter = 0;
    wake_me(duration, report);

    switch (test[0]) {
        case 'm':  // mix
            fd = create_fd();
            while (true) {
                close(dup(fd));
                getpid();
                getuid();
                umask(022);
                ++iter;
            }
            break;

        case 'c':  // close only
            fd = create_fd();
            while (true) {
                close(dup(fd));
                ++iter;
            }
            break;

        case 'g':  // getpid only
            while (true) {
                getpid();
                ++iter;
            }
            break;

        case 'e':  // exec
            while (true) {
                pid_t pid = fork();
                if (pid < 0) {
                    std::cerr << argv[0] << ": fork failed" << std::endl;
                    std::exit(1);
                } else if (pid == 0) {
                    execl("/bin/true", "/bin/true", nullptr);
                    std::cerr << argv[0] << ": exec /bin/true failed" << std::endl;
                    std::exit(1);
                } else {
                    if (waitpid(pid, nullptr, 0) < 0) {
                        std::cerr << argv[0] << ": waitpid failed" << std::endl;
                        std::exit(1);
                    }
                }
                ++iter;
            }
            break;

        default:
            return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

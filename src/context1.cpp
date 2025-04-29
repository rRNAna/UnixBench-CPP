/**
 * @file        context1.cpp
 * @brief       C++ refactored UnixBench arith test module
 * @author      rRNA
 * @version     3.4.0
 * @date        04-28-2025
 *
 * @details
 * This file is a C++ rewrite of context1.c from the original UnixBench project.
 * Original project address: https://github.com/kdlucas/byte-unixbench/tree/v5.1.3
 *
 *
 *
 */

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <exception>

#include <signal.h>
// Declare global variable for iteration count
unsigned long iter;

// Exception class for error handling
class PipeException : public std::exception
{
private:
    std::string message;

public:
    PipeException(const std::string &msg) : message(msg) {}
    const char *what() const noexcept override
    {
        return message.c_str();
    }
};

// Report function for alarm handler
void report(int)
{
    std::cerr << "COUNT|" << iter << "|1|lps" << std::endl;
    std::exit(0);
}

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: " << argv[0] << " duration" << std::endl;
            return 1;
        }

        int duration = std::atoi(argv[1]);
        unsigned long check;

        int p1[2], p2[2]; // Using array to handle pipe file descriptors

        if (pipe(p1) || pipe(p2))
        {
            throw PipeException("Pipe creation failed");
        }

        if (fork())
        { // Parent process
            signal(SIGALRM, report);
            alarm(duration); // Setup alarm
            usleep(100000);  // Give child a head start
            close(p1[0]);
            close(p2[1]);
            while (true)
            {
                if (write(p1[1], &iter, sizeof(iter)) != sizeof(iter))
                {
                    throw PipeException("Master write failed");
                }
                if (read(p2[0], &check, sizeof(check)) != sizeof(check))
                {
                    perror("read");
                    throw PipeException("Master read failed");
                }
                if (check != iter)
                {
                    std::cerr << "Master sync error: expected " << iter << ", got " << check << std::endl;
                    return 2;
                }
                iter++;
            }
        }
        else
        {             // Child process
            sleep(1); // Ensure parent is ready to write
            try
            {
                unsigned long iter1 = 0;
                close(p1[1]);
                close(p2[0]);
                while (true)
                {
                    ssize_t r = read(p1[0], &check, sizeof(check));
                    if (r == 0)
                    {
                        // std::cerr << "Slave: pipe closed by master" << std::endl;
                        std::exit(0); // Clean exit
                    }
                    if (r != sizeof(check))
                    {
                        perror("read");
                        throw PipeException("Slave read failed");
                    }
                    if (check != iter1) {
                        std::cerr << "Slave sync error: expected " << iter1 << ", got " << check << std::endl;
                        std::exit(2);
                    }
                    if (write(p2[1], &iter1, sizeof(iter1)) != sizeof(check))
                    {
                        throw PipeException("Slave write failed");
                    }
                    iter1++;
                }
            }
            catch (const PipeException &e)
            {
                std::cerr << "Child pipe error: " << e.what() << std::endl;
                std::exit(1);
            }
        }
    }
    catch (const PipeException &ex)
    {
        std::cerr << "Pipe exception: " << ex.what() << std::endl;
        return 1;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Unhandled exception: " << ex.what() << std::endl;
        return 1;
    }
}

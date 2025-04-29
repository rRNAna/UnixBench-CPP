/**
 * @file        fstime.cpp
 * @brief       C++ refactored UnixBench arith test module
 * @author      rRNA
 * @version     3.6.0
 * @date        04-28-2025
 *
 * @details
 * This file is a C++ rewrite of fstime.c from the original UnixBench project.
 * Original project address: https://github.com/kdlucas/byte-unixbench/tree/v5.1.3
 *
 *
 *
 */
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>

using namespace std;

//------------------- Constant -------------------
constexpr int SECONDS = 10;
constexpr int MAX_BUFSIZE = 8192;
constexpr int COUNTSIZE = 256;
constexpr int HALFCOUNT = COUNTSIZE / 2;
constexpr const char* FNAME0 = "dummy0";
constexpr const char* FNAME1 = "dummy1";

//------------------- Global variables ------------
int bufsize = 1024;           // The buffer size used for the test
int max_blocks = 2000;        // The maximum number of 1024-byte blocks in a file
int max_buffs = 2000;         // BUFSIZE number of blocks in the file
int count_per_k;              // Number of count units per 1024 bytes
int count_per_buf;            // Number of count units per bufsize

// Using static buffers
char buf[MAX_BUFSIZE];

int f; // File Descriptors：FNAME0
int g; // File Descriptors：FNAME1

// Control signal
int sigalarm = 0;

// Test scores
long read_score  = 1;
long write_score = 1;
long copy_score  = 1;

//------------------- Function declaration -----
void stop_count(int single_number);
void clean_up();
static double getFloatTime();
int w_test(int timeSecs);
int r_test(int timeSecs);
int c_test(int timeSecs);

//------------------- Main -------------------
int main(int argc, char* argv[]) {
    // Default run time and test type
    int seconds = SECONDS;
    char test = 'c';  // Default replication test

    int status = 0;

    // Processing command line arguments
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'c':
                case 'r':
                case 'w':
                    test = argv[i][1];
                    break;
                case 'b':
                    if (++i < argc) {
                        bufsize = std::atoi(argv[i]);
                    }
                    break;
                case 'm':
                    if (++i < argc) {
                        max_blocks = std::atoi(argv[i]);
                    }
                    break;
                case 't':
                    if (++i < argc) {
                        seconds = std::atoi(argv[i]);
                    }
                    break;
                case 'd':
                    if (++i < argc) {
                        if (chdir(argv[i]) < 0) {
                            perror("fstime: chdir");
                            exit(1);
                        }
                    }
                    break;
                default:
                    cerr << "Usage: fstime [-c|-r|-w] [-b <bufsize>] [-m <max_blocks>] [-t <seconds>]" << endl;
                    exit(2);
            }
        } else {
            cerr << "Usage: fstime [-c|-r|-w] [-b <bufsize>] [-m <max_blocks>] [-t <seconds>]" << endl;
            exit(2);
        }
    }

    // Check the validity of each parameter
    if (bufsize < COUNTSIZE || bufsize > 1024 * 1024) {
        cerr << "fstime: buffer size must be in range " << COUNTSIZE << "-" << 1024 * 1024 << endl;
        exit(3);
    }
    if (max_blocks < 1 || max_blocks > 1024 * 1024) {
        cerr << "fstime: max blocks must be in range " << 1 << "-" << 1024 * 1024 << endl;
        exit(3);
    }
    if (seconds < 1 || seconds > 3600) {
        cerr << "fstime: time must be in range " << 1 << "-" << 3600 << " seconds" << endl;
        exit(3);
    }

    max_buffs = max_blocks * 1024 / bufsize;
    count_per_k = 1024 / COUNTSIZE;
    count_per_buf = bufsize / COUNTSIZE;

    // Create or recreate the test files FNAME0 and FNAME1
    f = creat(FNAME0, 0600);
    if (f == -1) {
        perror("fstime: creat");
        exit(1);
    }
    close(f);

    g = creat(FNAME1, 0600);
    if (g == -1) {
        perror("fstime: creat");
        exit(1);
    }
    close(g);

    // Open the file for subsequent operations
    f = open(FNAME0, O_RDWR);
    if (f == -1) {
        perror("fstime: open");
        exit(1);
    }
    g = open(FNAME1, O_RDWR);
    if (g == -1) {
        perror("fstime: open");
        exit(1);
    }

    // Filling the buffer
    for (int i = 0; i < bufsize; ++i) {
        buf[i] = i & 0xff;
    }

    // Set the signal handler (SIGKILL cannot be captured, the original code logic is retained here)
    signal(SIGKILL, reinterpret_cast<void(*)(int)>(clean_up));

    // Run tests based on test type
    switch (test) {
        case 'w':
            status = w_test(seconds);
            break;
        case 'r':
            // First write 2 seconds of data for calibration
            w_test(2);
            status = r_test(seconds);
            break;
        case 'c':
            w_test(2);
            r_test(2);
            status = c_test(seconds);
            break;
        default:
            cerr << "fstime: unknown test '" << test << "'" << endl;
            exit(6);
    }

    if (status) {
        clean_up();
        exit(1);
    }

    clean_up();
    exit(0);
}

//------------------- Helper Functions ----------

// Get the current time (in floating point seconds) using gettimeofday
static double getFloatTime() {
    struct timeval t;
    if (gettimeofday(&t, nullptr) != 0) {
        perror("gettimeofday");
        exit(1);
    }
    return static_cast<double>(t.tv_sec) + static_cast<double>(t.tv_usec) / 1000000.0;
}

// Write test: continuously write data to the file within timeSecs seconds and calculate the write score
int w_test(int timeSecs) {
    unsigned long counted = 0L;
    unsigned long tmp;
    long f_blocks;
    double start, end;
    // Synchronous disk
    sync();
    sleep(2);
    sync();
    sleep(2);

    // Setting the timing signal
    sigalarm = 0;
    signal(SIGALRM, stop_count);
    alarm(timeSecs);

    start = getFloatTime();

    while (!sigalarm) {
        for (f_blocks = 0; f_blocks < max_buffs; ++f_blocks) {
            tmp = write(f, buf, bufsize);
            if (tmp != static_cast<unsigned long>(bufsize)) {
                if (errno != EINTR) {
                    perror("fstime: write");
                    return -1;
                }
                stop_count(0);
                counted += ((tmp + HALFCOUNT) / COUNTSIZE);
            } else {
                counted += count_per_buf;
            }
        }
        lseek(f, 0L, SEEK_SET); // rewind 文件指针
    }

    // Stop the timer
    end = getFloatTime();
    write_score = static_cast<long>( (static_cast<double>(counted)) / ((end - start) * count_per_k) );
    cout << "Write done: " << counted << " in " << (end - start)
         << ", score " << write_score << endl;

    cerr << "COUNT|" << write_score << "|0|KBps" << endl;
    cerr << "TIME|" << (end - start) << endl;

    return 0;
}

// Read test: continuously read data from the file within timeSecs seconds and calculate the read score
int r_test(int timeSecs) {
    unsigned long counted = 0L;
    unsigned long tmp;
    double start, end;
    // Synchronous disk
    sync();
    sleep(2);
    sync();
    sleep(2);

    // rewind file pointer
    errno = 0;
    lseek(f, 0L, SEEK_SET);

    // Setting the timing signal
    sigalarm = 0;
    signal(SIGALRM, stop_count);
    alarm(timeSecs);

    start = getFloatTime();

    while (!sigalarm) {
        tmp = read(f, buf, bufsize);
        if (tmp != static_cast<unsigned long>(bufsize)) {
            switch (errno) {
                case 0:
                case EINVAL:
                    lseek(f, 0L, SEEK_SET);
                    counted += ((tmp + HALFCOUNT) / COUNTSIZE);
                    continue;
                case EINTR:
                    stop_count(0);
                    counted += ((tmp + HALFCOUNT) / COUNTSIZE);
                    break;
                default:
                    perror("fstime: read");
                    return -1;
            }
        } else {
            counted += count_per_buf;
        }
    }

    // Stop the timer
    end = getFloatTime();
    read_score = static_cast<long>( (static_cast<double>(counted)) / ((end - start) * count_per_k) );
    cout << "Read done: " << counted << " in " << (end - start)
         << ", score " << read_score << endl;

    cerr << "COUNT|" << read_score << "|0|KBps" << endl;
    cerr << "TIME|" << (end - start) << endl;

    return 0;
}

// Copy test: continuously read and write data within timeSecs seconds, copy the file, and calculate the score
int c_test(int timeSecs) {
    unsigned long counted = 0L;
    unsigned long tmp;
    double start, end;

    sync();
    sleep(2);
    sync();
    sleep(1);

    // rewind file pointer
    errno = 0;
    lseek(f, 0L, SEEK_SET);

    // Setting the timing signal
    sigalarm = 0;
    signal(SIGALRM, stop_count);
    alarm(timeSecs);

    start = getFloatTime();

    while (!sigalarm) {
        tmp = read(f, buf, bufsize);
        if (tmp != static_cast<unsigned long>(bufsize)) {
            switch (errno) {
                case 0:
                case EINVAL:
                    lseek(f, 0L, SEEK_SET);
                    lseek(g, 0L, SEEK_SET);
                    continue;
                case EINTR:
                    counted += ((tmp * write_score) / (read_score + write_score) + HALFCOUNT) / COUNTSIZE;
                    stop_count(0);
                    break;
                default:
                    perror("fstime: copy read");
                    return -1;
            }
        } else {
            tmp = write(g, buf, bufsize);
            if (tmp != static_cast<unsigned long>(bufsize)) {
                if (errno != EINTR) {
                    perror("fstime: copy write");
                    return -1;
                }
                counted += ((tmp + (((bufsize - tmp) * write_score) / (read_score + write_score)) + HALFCOUNT) / COUNTSIZE);
                stop_count(0);
            } else {
                counted += count_per_buf;
            }
        }
    }

    // Stop the timer
    end = getFloatTime();
    copy_score = static_cast<long>( (static_cast<double>(counted)) / ((end - start) * count_per_k) );
    cout << "Copy done: " << counted << " in " << (end - start)
         << ", score " << copy_score << endl;

    cerr << "COUNT|" << copy_score << "|0|KBps" << endl;
    cerr << "TIME|" << (end - start) << endl;

    return 0;
}

// stop_count: called by SIGALRM signal, sets sigalarm flag
void stop_count(int single_number) {
    sigalarm = 1;
}

// clean_up: delete test files
void clean_up() {
    unlink(FNAME0);
    unlink(FNAME1);
}
/**
 * @file        time-pollong.cpp
 * @brief       C++ refactored UnixBench arith test module
 * @author      rRNA
 * @version     2.0.0
 * @date        04-28-2025
 *
 * @details
 * This file is a C++ rewrite of time-pollong.c from the original UnixBench project.
 * Original project address: https://github.com/kdlucas/byte-unixbench/tree/v5.1.3
 *
 *
 *
 */

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/time.h>
#include <sys/resource.h>
#ifdef HAS_POLL
  #include <poll.h>
#endif
#ifdef HAS_POLL2
  #include <linux/poll2.h>
#endif
#include <unistd.h>
#include <errno.h>
#include <vector>

using namespace std;

// Output to stdout if UNIXBENCH is defined, otherwise to stderr
#ifdef UNIXBENCH
  #define OUT cout
#else
  #define OUT cerr
#endif

constexpr int TRUE_VALUE = 1;
constexpr int FALSE_VALUE = 0;
#ifdef UNIXBENCH
  constexpr int MAX_ITERATIONS = 1000;
#else
  constexpr int MAX_ITERATIONS = 30;
#endif
constexpr int MAX_FDS = 40960;
#define CONST const
#define ERRSTRING strerror(errno)

// Type flag is defined as int
using flag = int;

//-------------------------------------------------------------------
// find_first_set_bit: Find the first set bit in a bit field
static inline int find_first_set_bit(CONST void *array, int size)
{
    int index = 0;
    unsigned long word;
    unsigned int ul_size = 8 * sizeof(unsigned long);
    CONST unsigned long *ul_array = static_cast<CONST unsigned long *>(array);

    // Find the first word that is not all zeros
    for (; (*ul_array == 0) && (index < size); index += ul_size, ++ul_array);
    // Find the first set bit in a word
    for (word = *ul_array; !(word & 1) && (index < size); ++index, word >>= 1);
    return index;
}

//-------------------------------------------------------------------
// find_next_set_bit: Find the first set bit after offset
static inline int find_next_set_bit(CONST void *array, int size, int offset)
{
    if (++offset >= size)
        return offset;
    int index = offset;
    unsigned long word;
    unsigned int ul_size = 8 * sizeof(unsigned long);
    CONST unsigned long *ul_array = static_cast<CONST unsigned long *>(array);

    // Jump to the long integer containing offset
    int tmp = offset / ul_size;
    ul_array += tmp;
    offset -= tmp * ul_size;
    if ((offset == 0) || (*ul_array == 0))
        return find_first_set_bit(ul_array, size - index) + index;

    // Check if there is a set bit in the current word
    if (((word = *ul_array) != 0) && ((word = word >> offset) != 0))
    {
        for (; (word & 1) == 0; word >>= 1, ++index);
        return index;
    }
    // Otherwise check the following word
    index += ul_size - offset;
    return find_first_set_bit(++ul_array, size - index) + index;
}
//-------------------------------------------------------------------

// Callback function structure
struct callback_struct {
    void (*input_func)(void *info);
    void (*output_func)(void *info);
    void (*exception_func)(void *info);
    void *info;
};

static int total_bits = 0;
static callback_struct callbacks[MAX_FDS];

// A simple test callback
static void test_func(void *info)
{
    ++total_bits;
}

#ifdef HAS_SELECT
//-------------------------------------------------------------------
// time_select: Test select call time
static void time_select(fd_set *input_fds, fd_set *output_fds,
                         fd_set *exception_fds, int max_fd, int num_iter,
                         long *times)
{
    int fd, count, nready;
    fd_set i_fds, o_fds, e_fds;
    struct timeval time1, time2, tv;

    // Pre-warming the cache
    memcpy(&i_fds, input_fds, sizeof(i_fds));
    memcpy(&o_fds, output_fds, sizeof(i_fds));
    memcpy(&e_fds, exception_fds, sizeof(i_fds));
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    select(max_fd + 1, &i_fds, &o_fds, &e_fds, &tv);
    for (count = 0; count < num_iter; ++count)
    {
        total_bits = 0;
        gettimeofday(&time1, nullptr);
        memcpy(&i_fds, input_fds, sizeof(i_fds));
        memcpy(&o_fds, output_fds, sizeof(i_fds));
        memcpy(&e_fds, exception_fds, sizeof(i_fds));
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        nready = select(max_fd + 1, &i_fds, &o_fds, &e_fds, &tv);
        if (nready == -1)
        {
            cerr << "Error selecting\t" << ERRSTRING << endl;
            exit(2);
        }
        if (nready < 1)
        {
            cerr << "Error: nready: " << nready << endl;
            exit(1);
        }
        // Handing exception_fds
        for (fd = find_first_set_bit(&e_fds, sizeof(e_fds) * 8); fd <= max_fd;
             fd = find_next_set_bit(&e_fds, sizeof(e_fds) * 8, fd))
        {
            callbacks[fd].exception_func(callbacks[fd].info);
        }
        // Handing input_fds
        for (fd = find_first_set_bit(&i_fds, sizeof(i_fds) * 8); fd <= max_fd;
             fd = find_next_set_bit(&i_fds, sizeof(i_fds) * 8, fd))
        {
            callbacks[fd].input_func(callbacks[fd].info);
        }
        // Handing output_fds
        for (fd = find_first_set_bit(&o_fds, sizeof(o_fds) * 8); fd <= max_fd;
             fd = find_next_set_bit(&o_fds, sizeof(o_fds) * 8, fd))
        {
            callbacks[fd].output_func(callbacks[fd].info);
        }
        gettimeofday(&time2, nullptr);
        times[count] = (time2.tv_sec - time1.tv_sec) * 1000000 +
                       (time2.tv_usec - time1.tv_usec);
    }
}
#endif  // HAS_SELECT

#ifdef HAS_POLL
//-------------------------------------------------------------------
// time_poll: Test poll call time
static void time_poll(struct pollfd *pollfd_array, int start_index,
                      int num_to_test, int num_iter, long *times)
{
    short revents;
    int fd, count, nready;
    struct timeval time1, time2;
    struct pollfd *pollfd_ptr, *stop_pollfd;

    // Pre-warming the cache
    poll(pollfd_array + start_index, num_to_test, 0);
    for (count = 0; count < num_iter; ++count)
    {
        total_bits = 0;
        gettimeofday(&time1, nullptr);
        nready = poll(pollfd_array + start_index, num_to_test, 0);
        if (nready == -1)
        {
            cerr << "Error polling\t" << ERRSTRING << endl;
            exit(2);
        }
        if (nready < 1)
        {
            cerr << "Error: nready: " << nready << endl;
            exit(1);
        }
        stop_pollfd = pollfd_array + start_index + num_to_test;
        for (pollfd_ptr = pollfd_array + start_index; true; ++pollfd_ptr)
        {
            if (pollfd_ptr->revents == 0) continue;
            revents = pollfd_ptr->revents;
            fd = pollfd_ptr->fd;
            if (revents & POLLPRI)
                callbacks[fd].exception_func(callbacks[fd].info);
            if (revents & POLLIN)
                callbacks[fd].input_func(callbacks[fd].info);
            if (revents & POLLOUT)
                callbacks[fd].output_func(callbacks[fd].info);
            if (--nready == 0) break;
        }
        gettimeofday(&time2, nullptr);
        times[count] = (time2.tv_sec - time1.tv_sec) * 1000000 +
                       (time2.tv_usec - time1.tv_usec);
    }
}
#endif  // HAS_POLL

#ifdef HAS_POLL2
//-------------------------------------------------------------------
// time_poll2: Test poll2 call time
static void time_poll2(struct poll2ifd *poll2ifd_array, int start_index,
                       int num_to_test, int num_iter, long *times)
{
    short revents;
    int fd, count, nready, i;
    struct timeval time1, time2;
    struct poll2ofd poll2ofd_array[MAX_FDS];

    // Pre-warming the cache
    poll2(poll2ifd_array + start_index, poll2ofd_array, num_to_test, 0);
    for (count = 0; count < num_iter; ++count)
    {
        total_bits = 0;
        gettimeofday(&time1, nullptr);
        nready = poll2(poll2ifd_array + start_index, poll2ofd_array, num_to_test, 0);
        if (nready == -1)
        {
            times[count] = -1;
            if (errno == ENOSYS) return;
            cerr << "Error calling poll2(2)\t" << ERRSTRING << endl;
            exit(2);
        }
        if (nready < 1)
        {
            cerr << "Error: nready: " << nready << endl;
            exit(1);
        }
        for (i = 0; i < nready; ++i)
        {
            revents = poll2ofd_array[i].revents;
            fd = poll2ofd_array[i].fd;
            if (revents & POLLPRI)
                callbacks[fd].exception_func(callbacks[fd].info);
            if (revents & POLLIN)
                callbacks[fd].input_func(callbacks[fd].info);
            if (revents & POLLOUT)
                callbacks[fd].output_func(callbacks[fd].info);
        }
        gettimeofday(&time2, nullptr);
        times[count] = (time2.tv_sec - time1.tv_sec) * 1000000 +
                       (time2.tv_usec - time1.tv_usec);
    }
}
#endif  // HAS_POLL2

//-------------------------------------------------------------------
// main function
int main(int argc, char *argv[]) {
    flag failed = FALSE;
    flag verbose = FALSE;
    int first_fd = -1;
    int fd, max_fd = 0, count, total_fds = 0;
    int num_to_test, num_active;
#ifdef UNIXBENCH
    int max_iter = 1000;
#else
    int max_iter = 10;
#endif
#ifdef HAS_SELECT
    long select_total = 0;
    fd_set input_fds, output_fds, exception_fds;
    long select_times[MAX_ITERATIONS];
#endif
#ifdef HAS_POLL
    int start_index = 0;
    long poll_total = 0;
    struct pollfd pollfd_array[MAX_FDS];
    long poll_times[MAX_ITERATIONS];
#endif
#ifdef HAS_POLL2
    long poll2_total = 0;
    struct poll2ifd poll2ifd_array[MAX_FDS];
    long poll2_times[MAX_ITERATIONS];
#endif

#ifdef HAS_SELECT
    FD_ZERO(&input_fds);
    FD_ZERO(&output_fds);
    FD_ZERO(&exception_fds);
#endif
#ifdef HAS_POLL
    memset(pollfd_array, 0, sizeof(pollfd_array));
#endif
    // Allocate file descriptors
    total_fds = 0;
    while (!failed) {
        fd = dup(1);
        if (fd == -1) {
            if (errno != EMFILE) {
                cerr << "Error dup()ing\t" << ERRSTRING << endl;
                exit(1);
            }
            failed = TRUE;
            continue;
        }
        if (fd >= MAX_FDS) {
            cerr << "File descriptor: " << fd
                 << " larger than max: " << MAX_FDS - 1 << endl;
            exit(1);
        }
        callbacks[fd].input_func = test_func;
        callbacks[fd].output_func = test_func;
        callbacks[fd].exception_func = test_func;
        callbacks[fd].info = nullptr;
        if (fd > max_fd)
            max_fd = fd;
        if (first_fd < 0)
            first_fd = fd;
#ifdef HAS_POLL
        pollfd_array[fd].fd = fd;
        pollfd_array[fd].events = 0;
#endif
#ifdef HAS_POLL2
        poll2ifd_array[fd].fd = fd;
        poll2ifd_array[fd].events = 0;
#endif
        total_fds++;
    }
    total_fds = max_fd + 1;
    // Processing command line arguments
    if (argc > 5) {
        cerr << "Usage:\ttime-polling [num_iter] [num_to_test] [num_active] [-v]" << endl;
        exit(1);
    }
    if (argc > 1)
        max_iter = std::atoi(argv[1]);
    if (max_iter > MAX_ITERATIONS) {
        cerr << "num_iter too large" << endl;
        exit(1);
    }
    if (argc > 2)
        num_to_test = std::atoi(argv[2]);
    else
        num_to_test = total_fds - first_fd;
    if (argc > 3)
        num_active = std::atoi(argv[3]);
    else
        num_active = 1;
    if (argc > 4) {
        if (string(argv[4]) != "-v") {
            cerr << "Usage:\ttime-polling [num_iter] [num_to_test] [num_active] [-v]" << endl;
            exit(1);
        }
        verbose = TRUE;
    }
    if (num_to_test > total_fds - first_fd)
        num_to_test = total_fds - first_fd;
    if (num_active > total_fds - first_fd)
        num_active = total_fds - first_fd;

#ifdef HAS_SELECT
    for (fd = total_fds - num_to_test; fd < total_fds; ++fd) {
        FD_SET(fd, &exception_fds);
        FD_SET(fd, &input_fds);
    }
#endif
#ifdef HAS_POLL
    for (fd = total_fds - num_to_test; fd < total_fds; ++fd) {
        pollfd_array[fd].events = POLLPRI | POLLIN;
    }
#endif
#ifdef HAS_POLL2
    for (fd = total_fds - num_to_test; fd < total_fds; ++fd) {
        poll2ifd_array[fd].events = POLLPRI | POLLIN;
    }
#endif
#ifdef HAS_SELECT
    for (fd = total_fds - num_active; fd < total_fds; ++fd) {
        FD_SET(fd, &output_fds);
    }
#endif
#ifdef HAS_POLL
    for (fd = total_fds - num_active; fd < total_fds; ++fd) {
        pollfd_array[fd].events |= POLLOUT;
    }
#endif
#ifdef HAS_POLL2
    for (fd = total_fds - num_active; fd < total_fds; ++fd) {
        poll2ifd_array[fd].events |= POLLOUT;
    }
#endif

    OUT << "Num fds: " << total_fds << ", polling descriptors "
        << total_fds - num_to_test << "-" << max_fd << "\n";

#ifdef HAS_SELECT
    time_select(&input_fds, &output_fds, &exception_fds, max_fd, max_iter, select_times);
#endif
#ifdef HAS_POLL
    start_index = total_fds - num_to_test;
    time_poll(pollfd_array, start_index, num_to_test, max_iter, poll_times);
#endif
#ifdef HAS_POLL2
    start_index = total_fds - num_to_test;
    time_poll2(poll2ifd_array, start_index, num_to_test, max_iter, poll2_times);
#endif

    OUT << "All times in microseconds\n";
    OUT << "ITERATION\t";
#ifdef HAS_SELECT
    OUT << "select(2)" << "\t";
#endif
#ifdef HAS_POLL
    OUT << "poll(2)" << "\t";
#endif
#ifdef HAS_POLL2
    if (poll2_times[0] >= 0) OUT << "poll2(2)" << "\t";
#endif
    OUT << "\n";

    for (count = 0; count < max_iter; ++count) {
        if (verbose) OUT << count << "\t\t";
#ifdef HAS_SELECT
        if (verbose) OUT << select_times[count] << "\t";
        select_total += select_times[count];
#endif
#ifdef HAS_POLL
        if (verbose) OUT << poll_times[count] << "\t";
        poll_total += poll_times[count];
#endif
#ifdef HAS_POLL2
        if (verbose && (poll2_times[0] >= 0))
            OUT << poll2_times[count] << "\t";
        poll2_total += poll2_times[count];
#endif
        if (verbose) OUT << "\n";
    }
    OUT << "\n\naverage\t\t";
#ifdef HAS_SELECT
    OUT << (select_total / max_iter) << "\t";
#endif
#ifdef HAS_POLL
    OUT << (poll_total / max_iter) << "\t";
#endif
#ifdef HAS_POLL2
    if (poll2_times[0] >= 0)
        OUT << (poll2_total / max_iter) << "\t";
#endif
    OUT << "\nPer fd\t\t";
#ifdef HAS_SELECT
    OUT << ((float)select_total / (float)max_iter / (float)num_to_test) << "\t";
#ifdef UNIXBENCH
    cerr << "lps\t" << 1000000.0 * max_iter * num_to_test / select_total
         << "\t" << ((float)select_total / 1000000.0) << "\n";
#endif
#endif
#ifdef HAS_POLL
    OUT << ((float)poll_total / (float)max_iter / (float)num_to_test) << "\t";
#ifdef UNIXBENCH
    cerr << "lps\t" << 1000000.0 * max_iter * num_to_test / poll_total
         << "\t" << ((float)poll_total / 1000000.0) << "\n";
#endif
#endif
#ifdef HAS_POLL2
    if (poll2_times[0] >= 0) {
        OUT << ((float)poll2_total / (float)max_iter / (float)num_to_test) << "\t";
#ifdef UNIXBENCH
        cerr << "lps\t" << 1000000.0 * max_iter * num_to_test / poll2_total
             << "\t" << ((float)poll2_total / 1000000.0) << "\n";
#endif
    }
#endif
    OUT << "<- the most important value\n";

    return 0;
}
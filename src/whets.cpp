/**
 * @file        whets.cpp
 * @brief       C++ refactored UnixBench arith test module
 * @author      rRNA
 * @version     2.0.0
 * @date        04-28-2025
 *
 * @details
 * This file is a C++ rewrite of whets.c from the original UnixBench project.
 * Original project address: https://github.com/kdlucas/byte-unixbench/tree/v5.1.3
 *
 *
 *
 */

#include <cmath>      // for sin, cos, atan, exp, log, sqrt
#include <iostream>   // for std::cout, std::cerr
#include <string>    // for std::string
#include <cstdlib>    // for exit, atoi
#include <array>
#include <chrono>
#include <vector>
#include <iomanip>

using namespace std;

// Include iostream for debugging output

#ifdef DP
using SPDP = double;
constexpr const char* Precision = "Double";
#else
using SPDP = float;
constexpr const char* Precision = "Single";
#endif

// Function prototypes:
void whetstones(long xtra, long x100, int calibrate);
void pa(SPDP e[4], SPDP t, SPDP t2);
void po(SPDP e1[4], long j, long k, long l);
void p3(SPDP *x, SPDP *y, SPDP *z, SPDP t, SPDP t1, SPDP t2);
void pout(const string& title, float ops, int type, SPDP checknum,
    SPDP time, int calibrate, int section);

// Static global variables:
static vector<SPDP> loop_time(9);
static vector<SPDP> loop_mops(9);
static vector<SPDP> loop_mflops(9);
static SPDP TimeUsed;
static SPDP mwips;
static array<string, 9> headings;
static SPDP Check;
static vector<SPDP> results(9);

// Define a function that returns the number of seconds since a fixed point, of type double
double dtime() {
    static auto start = chrono::high_resolution_clock::now();
    auto now = chrono::high_resolution_clock::now();
    return chrono::duration<double>(now - start).count();
}

// main
int main(int argc, char *argv[]) {
    cout << "Starting the Whetstone Double-Precision Benchmark..." << endl;

    int count = 10, calibrate = 1;
    long xtra = 1;
    long x100 = 100;
    int duration = 100;
    bool getinput = true;
    vector<string> general(10, " ");
    string compiler = " ";
    string options = " ";

    cout << "##########################################" << endl;
    cout << Precision << " Precision C/C++ Whetstone Benchmark" << endl << endl;

    if (argc > 1) {
        if (argv[1][0] == 'N' || argv[1][0] == 'n') {
            getinput = false;
        }
    }
    if (!getinput) {
        cout << "No run time input data" << endl << endl;
    }

    cout << "Calibrate" << endl;
    do {
        TimeUsed = 0;
        whetstones(xtra, x100, calibrate);
        printf("%11.2f Seconds %10.0f   Passes (x 100)\n", TimeUsed, (SPDP)(xtra));
        calibrate++;
        count--;
        if (TimeUsed > 2.0) {
            count = 0;
        } else {
            xtra = xtra * 5;
        }
    } while (count > 0);

    if (TimeUsed > 0)
        xtra = (long)((SPDP)(duration * xtra) / TimeUsed);
    if (xtra < 1)
        xtra = 1;

    calibrate = 0;

    cout << "\nUse " << xtra << "  passes (x 100)" << endl;

    cout << "\n          " << Precision << " Precision C/C++ Whetstone Benchmark" << endl;

    cout << "\nLoop content                  Result              MFLOPS      MOPS   Seconds" << endl << endl;

    TimeUsed = 0;
    whetstones(xtra, x100, calibrate);

    cout << "\nMWIPS            ";
    if (TimeUsed > 0) {
        mwips = static_cast<float>(xtra) * static_cast<float>(x100) / (10 * TimeUsed);
    } else {
        mwips = 0;
    }
    printf("%39.3f%19.3f\n\n", mwips, TimeUsed);

    if (Check == 0)
        cout << "Wrong answer  " << endl;

    cout << "\n";
    cout << "A new results file will have been created in the same directory as the" << endl;
    cout << ".EXE files if one did not already exist. If you made a mistake on input," << endl;
    cout << "you can use a text editor to correct it, delete the results or copy" << endl;
    cout << "them to a different file name. If you intend to run multiple tests you" << endl;
    cout << "you may wish to rename WHETS.RES with a more informative title." << endl << endl;
    cout << "Please submit feedback and results files to aburto@nosc.mil or to" << endl;
    cout << "Roy_Longbottom@compuserve.com" << endl << endl;

    std::cout << "COUNT|" << std::fixed << std::setprecision(3) << mwips << "|1|mwips" << std::endl;

    return 0;
}

void whetstones(long xtra, long x100, int calibrate) {
    cout << "Entering whetstones function with xtra: " << xtra << ", x100: " << x100 << ", calibrate: " << calibrate << endl;

    long n1, n2, n3, n4, n5, n6, n7, n8, i, ix, n1mult;
    SPDP x, y, z;
    long j, k, l;
    SPDP e1[4], timea, timeb;

    SPDP t = 0.49999975;
    SPDP t0 = t;
    SPDP t1 = 0.50000025;
    SPDP t2 = 2.0;

    Check = 0.0;

    n1 = 12 * x100;
    n2 = 14 * x100;
    n3 = 345 * x100;
    n4 = 210 * x100;
    n5 = 32 * x100;
    n6 = 899 * x100;
    n7 = 616 * x100;
    n8 = 93 * x100;
    n1mult = 10;

    // Section 1, Array elements
    e1[0] = 1.0;
    e1[1] = -1.0;
    e1[2] = -1.0;
    e1[3] = -1.0;
    timea = dtime();
    for (ix = 0; ix < xtra; ix++) {
        cout << "Iteration: " << ix << " of whetstones loop" << endl;
        for (i = 0; i < n1 * n1mult; i++) {
            if (i % 100 == 0) {  // Reduce the frequency of debug output
                cout << "At loop n1, iteration: " << i << ", e1[0]: " << e1[0] << endl;
            }
            e1[0] = (e1[0] + e1[1] + e1[2] - e1[3]) * t;
            e1[1] = (e1[0] + e1[1] - e1[2] + e1[3]) * t;
            e1[2] = (e1[0] - e1[1] + e1[2] + e1[3]) * t;
            e1[3] = (-e1[0] + e1[1] + e1[2] + e1[3]) * t;
        }
        t = 1.0 - t;
    }
    t = t0;
    timeb = (dtime() - timea) / (SPDP)(n1mult);
    pout("N1 floating point", (float)(n1 * 16) * (float)(xtra),
         1, e1[3], timeb, calibrate, 1);

    // Section 2, Array as parameter
    timea = dtime();
    for (ix = 0; ix < xtra; ix++) {
        cout << "Calling function pa with parameters..." << endl;
        for (i = 0; i < n2; i++) {
            pa(e1, t, t2);
        }
        t = 1.0 - t;
    }
    t = t0;
    timeb = dtime() - timea;
    cout << "Returned from pa function." << endl;
    pout("N2 floating point", (float)(n2 * 96) * (float)(xtra),
         1, e1[3], timeb, calibrate, 2);

    // Section 3, Conditional jumps
    j = 1;
    timea = dtime();
    for (ix = 0; ix < xtra; ix++) {
        for (i = 0; i < n3; i++) {
            if(j==1)
                j = 2;
            else
                j = 3;
            if(j>2)
                j = 0;
            else
                j = 1;
            if(j<1)
                j = 1;
            else
                j = 0;
        }
    }
    timeb = dtime() - timea;
    pout("N3 if then else", (float)(n3 * 3) * (float)(xtra),
         2, (SPDP)(j), timeb, calibrate, 3);

    // Section 4, Integer arithmetic
    j = 1;
    k = 2;
    l = 3;
    timea = dtime();
    for (ix = 0; ix < xtra; ix++) {
        for (i = 0; i < n4; i++) {
            j = j * (k - j) * (l - k);
            k = l * k - (l - j) * k;
            l = (l - k) * (k + j);
            e1[l - 2] = j + k + l;
            e1[k - 2] = j * k * l;
        }
    }
    timeb = dtime() - timea;
    x = e1[0] + e1[1];
    pout("N4 fixed point", (float)(n4 * 15) * (float)(xtra),
         2, x, timeb, calibrate, 4);

    // Section 5, Trig functions
    x = 0.5;
    y = 0.5;
    timea = dtime();
    for (ix = 0; ix < xtra; ix++) {
        for (i = 1; i < n5; i++) {
            x = t * atan(t2 * sin(x) * cos(x) / (cos(x + y) + cos(x - y) - 1.0));
            y = t * atan(t2 * sin(y) * cos(y) / (cos(x + y) + cos(x - y) - 1.0));
        }
        t = 1.0 - t;
    }
    t = t0;
    timeb = dtime() - timea;
    pout("N5 sin,cos etc.", (float)(n5 * 26) * (float)(xtra),
         2, y, timeb, calibrate, 5);

    // Section 6, Procedure calls
    x = 1.0;
    y = 1.0;
    z = 1.0;
    timea = dtime();
    for (ix = 0; ix < xtra; ix++) {
        for (i = 0; i < n6; i++) {
            p3(&x, &y, &z, t, t1, t2);
        }
    }
    timeb = dtime() - timea;
    pout("N6 floating point", (float)(n6 * 6) * (float)(xtra),
         1, z, timeb, calibrate, 6);

    // Section 7, Array references
    j = 0;
    k = 1;
    l = 2;
    e1[0] = 1.0;
    e1[1] = 2.0;
    e1[2] = 3.0;
    timea = dtime();
    for (ix = 0; ix < xtra; ix++) {
        for (i = 0; i < n7; i++) {
            po(e1, j, k, l);
        }
    }
    timeb = dtime() - timea;
    pout("N7 assignments", (float)(n7 * 3) * (float)(xtra),
         2, e1[2], timeb, calibrate, 7);

    // Section 8, Standard functions
    x = 0.75;
    timea = dtime();
    for (ix = 0; ix < xtra; ix++) {
        for (i = 0; i < n8; i++) {
            x = sqrt(exp(log(x) / t1));
        }
    }
    timeb = dtime() - timea;
    pout("N8 exp,sqrt etc.", (float)(n8 * 4) * (float)(xtra),
         2, x, timeb, calibrate, 8);
    return;
}

void pa(SPDP e[4], SPDP t, SPDP t2) {
    long j;
    for(j = 0; j < 6; j++) {
        e[0] = (e[0] + e[1] + e[2] - e[3]) * t;
        e[1] = (e[0] + e[1] - e[2] + e[3]) * t;
        e[2] = (e[0] - e[1] + e[2] + e[3]) * t;
        e[3] = (-e[0] + e[1] + e[2] + e[3]) / t2;
    }
    return;
}

void po(SPDP e1[4], long j, long k, long l) {
    e1[j] = e1[k];
    e1[k] = e1[l];
    e1[l] = e1[j];
    return;
}

void p3(SPDP *x, SPDP *y, SPDP *z, SPDP t, SPDP t1, SPDP t2) {
    *x = *y;
    *y = *z;
    *x = t * (*x + *y);
    *y = t1 * (*x + *y);
    *z = (*x + *y) / t2;
    return;
}

void pout(const string& title, float ops, int type, SPDP checknum,
    SPDP time, int calibrate, int section) {
    SPDP mops, mflops;
    Check = Check + checknum;
    loop_time[section] = time;
    headings[section] = title;
    TimeUsed = TimeUsed + time;
    if (calibrate == 1) {
        results[section] = checknum;
    }
    if (calibrate == 0) {
        printf("%s %24.17f    ", headings[section].c_str(), results[section]);
        if (type == 1) {
            if (time > 0) {
                mflops = ops / (1000000L * time);
            } else {
                mflops = 0;
            }
            loop_mops[section] = 99999;
            loop_mflops[section] = mflops;
            printf(" %9.3f          %9.3f\n", loop_mflops[section], loop_time[section]);
        } else {
            if (time > 0) {
                mops = ops / (1000000L * time);
            } else {
                mops = 0;
            }
            loop_mops[section] = mops;
            loop_mflops[section] = 0;
            printf("            %9.3f%9.3f\n", loop_mops[section], loop_time[section]);
        }
    }
    return;
}
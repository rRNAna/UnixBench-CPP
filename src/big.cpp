/**
 * @file        big.cpp
 * @brief       C++ refactored UnixBench arith test module
 * @author      rRNA
 * @version     3.5.0
 * @date        6-10-2025
 *
 * @details
 * This file is a C++ rewrite of big.c from the original UnixBench project.
 * Original project address: https://github.com/kdlucas/byte-unixbench/tree/v5.1.3
 *
 * This module is used to manage stress test coordination tasks, such as multi-user
 * simulation, multi-process execution, logging, etc.
 */
#include "big.hpp"

// 所有实现函数放在 big.cpp，只保留逻辑实现部分，无 main()

#include <iostream>
#include <fstream>
#include <sstream>
#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <cstring>

float threshold;
float estimatedRate = DEF_RATE;
int numUsers = 0;
int firstUser = 0;
int exitStatus = 0;
int sigpipe = 0;

std::vector<Work> works;
std::vector<Child> children;

void getWork() {
    std::string line;
    while (std::getline(std::cin, line)) {
        if(line.empty()) continue;
        std::istringstream iss(line);
        Work work;
        if(!(iss >> work.cmd))
            continue;
        std::string token;
        while(iss >> token) {
            if(token == "<") {
                std::string filename;
                if(iss >> filename) {
                    std::ifstream infile(filename);
                    if(!infile) {
                        std::cerr << "cannot open input file (" << filename << ") for job\n";
                        exit(4);
                    }
                    std::string fileContent, fileLine;
                    while(std::getline(infile, fileLine)) {
                        fileContent += fileLine + "\n";
                        if(fileLine.rfind("C=", 0) == 0) {
                            work.outputFile = fileLine.substr(2);
                        }
                    }
                    work.input = fileContent;
                }
            } else {
                work.args.push_back(token);
            }
        }
        works.push_back(work);
    }
}

void wrapUp(const std::string& reason) {
    std::cerr << reason << std::endl;
    for (auto& child : children) {
        if (child.pid > 0 && kill(child.pid, SIGKILL) != -1) {
            std::cerr << "User " << firstUser << " job " << child.currentJob << " pid " << child.pid << " killed off\n";
        }
    }
    exit(exitStatus);
}

void onAlarm(int) {
    threshold += estimatedRate;
    std::cerr << "[SIGALRM] threshold increased to: " << threshold << std::endl;
    std::signal(SIGALRM, onAlarm);
    alarm(GRANULE);
}

void pipeError(int) {
    sigpipe++;
}

void grunt() {
    exitStatus = 4;
    wrapUp("Timed out waiting for jobs to finish ...");
}

void fatal(const std::string& message) {
    std::cerr << message << std::endl;
    exit(4);
}
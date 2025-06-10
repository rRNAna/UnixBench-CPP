//
// Created by rRNA on 25-6-10.
// v1.0.0
//

#ifndef BIG_HPP
#define BIG_HPP

#pragma once

#include <string>
#include <vector>

// Constants
constexpr float DEF_RATE = 5.0;
constexpr int GRANULE = 5;
constexpr int CHUNK = 60;
constexpr int MAXCHILD = 12;
constexpr int MAXWORK = 10;

// Structs
struct Work {
    std::string cmd;
    std::vector<std::string> args;
    std::string input;
    std::string outputFile;
};

struct Child {
    int xmit = 0;
    std::string inputBuffer;
    int inputPos = 0;
    int fd = 0;
    int pid = 0;
    int firstJob = 0;
    int currentJob = 0;
    const char* line = nullptr;
};

// Global variables
extern float threshold;
extern float estimatedRate;
extern int numUsers;
extern int firstUser;
extern int exitStatus;
extern int sigpipe;
extern std::vector<Work> works;
extern std::vector<Child> children;

// Function declarations
void wrapUp(const std::string& reason);
void onAlarm(int);
void pipeError(int);
void grunt();
void getWork();
void fatal(const std::string& message);

#endif //BIG_HPP
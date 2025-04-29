/**
 * @file        big.cpp
 * @brief       C++ refactored UnixBench arith test module
 * @author      rRNA
 * @version     3.4.0
 * @date        04-28-2025
 *
 * @details
 * This file is a C++ rewrite of big.c from the original UnixBench project.
 * Original project address: https://github.com/kdlucas/byte-unixbench/tree/v5.1.3
 *
 * This module is used to manage stress test coordination tasks, such as multi-user
 * simulation, multi-process execution, logging, etc.
 */

#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <csignal>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <fstream>
#include <sstream>

#define DEF_RATE 5.0
#define GRANULE  5
#define CHUNK    60
#define MAXCHILD 12
#define MAXWORK  10

void wrapUp(const std::string& reason);
void onAlarm(int);
void pipeError(int);
void grunt();
void getWork();
void fatal(const std::string& message);

float threshold;
float estimatedRate = DEF_RATE;
int numUsers = 0;   // number of concurrent users to be simulated by this process.
int firstUser = 0;  // ordinal identification of first user for this process.
int exitStatus = 0; // returned to parent
int sigpipe = 0;    // pipe write error flag

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
    const char* line = nullptr; // New member to track current line start
};

std::vector<Work> works;
std::vector<Child> children;

int main(int argc, char* argv[]) {
    std::string prog = argv[0];
    int fcopy = 0;

    // Process command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg[0] == '-') {
            switch (arg[1]) {
                case 'r':
                    estimatedRate = std::stof(argv[++i]);
                    if (estimatedRate <= 0) {
                        std::cerr << prog << ": bad rate, reset to " << DEF_RATE << " chars/sec\n";
                        estimatedRate = DEF_RATE;
                    }
                    break;
                case 'c':
                    fcopy = open(argv[++i], O_WRONLY);
                    if (fcopy < 0) fcopy = creat(argv[i], 0600);
                    if (fcopy < 0) {
                        std::cerr << prog << ": cannot open copy file '" << argv[i] << "'\n";
                        exit(2);
                    }
                    lseek(fcopy, 0L, 2);
                    break;
                default:
                    std::cerr << prog << ": bad flag '" << arg[1] << "'\n";
                    exit(4);
            }
        } else {
            numUsers = std::stoi(arg);
            if (numUsers < 1) {
                std::cerr << prog << ": impossible nusers (" << numUsers << ")\n";
                exit(4);
            }
        }
    }

    getWork();

    // Random seed initialization, matching big.c logic
    srand(time(0));

    int master = 1;
    firstUser = MAXCHILD;
    while (numUsers > MAXCHILD) {
        int nchild = (numUsers >= 2 * MAXCHILD) ? MAXCHILD : (numUsers - MAXCHILD);
        pid_t l = fork();
        if (l < 0) {
            fatal("** clone fork failed **");
            break;
        } else if (l > 0) {
            // Master process
            numUsers -= nchild;
            firstUser += MAXCHILD;
            continue;
        } else {
            // Clone process
            char logname[32];
            snprintf(logname, sizeof(logname), "masterlog.%02d", firstUser / MAXCHILD);
            freopen(logname, "w", stderr);
            master = 0;
            numUsers = nchild;
            break;
        }
    }
    if (master)
        firstUser = 0;

    std::signal(SIGALRM, onAlarm);
    std::signal(SIGPIPE, pipeError);
    alarm(GRANULE);

    // Main processing logic
    for (size_t i = 0; i < works.size(); ++i) {
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            std::cerr << prog << ": pipe error\n";
            wrapUp("Pipe creation failed");
        }

        pid_t pid = fork();
        if (pid < 0) {
            std::cerr << prog << ": fork error\n";
            wrapUp("Fork failed");
        } else if (pid == 0) {
            // Child process
            close(pipefd[1]); // Close write end of the pipe
            dup2(pipefd[0], STDIN_FILENO); // Redirect stdin
            close(pipefd[0]); // Close original pipe read end
            for (int fd = 3; fd < 24; fd++) close(fd);

            {
                char logname[32];
                snprintf(logname, sizeof(logname), "userlog.%02zu", firstUser + i);
                if (freopen(logname, "w", stderr) == NULL)
                    std::cerr << prog << ": cannot open user log " << logname << "\n";
            }
            if (!works[i].outputFile.empty()) {
                if (freopen(works[i].outputFile.c_str(), "w", stdout) == NULL)
                    std::cerr << prog << ": cannot open output file " << works[i].outputFile << "\n";
            }

            std::vector<char*> execArgs;
            execArgs.push_back(const_cast<char*>(works[i].cmd.c_str()));
            for (const auto& arg : works[i].args) {
                execArgs.push_back(const_cast<char*>(arg.c_str()));
            }
            execArgs.push_back(nullptr);

            execv(execArgs[0], execArgs.data());
            std::cerr << prog << ": execv error\n";
            exit(1);
        } else {
            // Parent process
            close(pipefd[0]); // Close read end of the pipe
            Child child;
            child.pid = pid;
            child.fd = pipefd[1];
            child.currentJob = i;
            children.push_back(child);
        }
    }

    // Parent process: send work.input contents to each child process, simulating rhythm control.
    for (size_t i = 0; i < children.size(); ++i) {
        children[i].inputBuffer = works[children[i].currentJob].input;
        children[i].inputPos = 0;
        children[i].line = children[i].inputBuffer.c_str(); // Initialize line pointer
    }
    int done = 0;
    int output = 0;
    threshold = 0;
    for (size_t i = 0; i < children.size(); ++i) {
        if (children[i].inputBuffer.empty())
            done++;
    }
    threshold = estimatedRate * numUsers * GRANULE;
    estimatedRate = threshold;
    // std::signal(SIGALRM, onAlarm);
    // std::signal(SIGPIPE, pipeError);
    alarm(GRANULE);
    while (done < (int)children.size()) {
        for (size_t i = 0; i < children.size(); ++i) {
            Child &child = children[i];
            if (child.xmit >= (int)child.inputBuffer.size()) continue;

            const char* buffer = child.inputBuffer.c_str();
            int remaining = child.inputBuffer.size() - child.xmit;
            int l = rand() % CHUNK + 1;
            if (l > remaining) l = remaining;
            const char* chunk_start = buffer + child.inputPos;
            const char* chunk_end = chunk_start + l;

            const char* p = chunk_start;
            while (p < chunk_end) {
                if (*p == '\n' || (p == chunk_end - 1 && child.xmit + (p - chunk_start + 1) >= (int)child.inputBuffer.size())) {
                    int len = p - child.line + 1;
                    if (write(child.fd, child.line, len) != len) {
                        if (sigpipe)
                            wrapUp("** SIGPIPE error **");
                        else
                            wrapUp("** write error **");
                    }
                    if (fcopy > 0)
                        write(fcopy, child.line, len);
                    child.line = p + 1;
                }
                p++;
            }

            child.inputPos += l;
            child.xmit += l;
            output += l;

            if (child.xmit >= (int)child.inputBuffer.size()) {
                done++;
                close(child.fd);
            }
        }
        while (output > threshold) {
            // 调试
            std::cerr << "[PAUSE] output = " << output << ", threshold = " << threshold << ", done = " << done << std::endl;
            std::cerr.flush();
            //
            pause();
        }
    }
    alarm(1200);
    int status;
    pid_t pid;
    while ((pid = wait(&status)) > 0) {
        // Optionally log child's termination status here.
    }
    alarm(0);
    if (fcopy > 0) close(fcopy); // Close fcopy if opened
    wrapUp("Finished waiting ...");

    return 0;
}

void getWork() {
    std::string line;
    while (std::getline(std::cin, line)) {
        if(line.empty()) continue;
        std::istringstream iss(line);
        Work work;
        // Extract command name and assign to work.cmd
        if(!(iss >> work.cmd))
            continue;
        std::string token;
        while(iss >> token) {
            if(token == "<") {
                // Next token is input file name; open and read file content
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
                        // If a line starts with "C=", extract the output file path
                        if(fileLine.rfind("C=", 0) == 0) {
                            work.outputFile = fileLine.substr(2);
                        }
                    }
                    work.input = fileContent;
                }
            } else {
                // Add token as an argument
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
    // 调试
    std::cerr << "[SIGALRM] threshold increased to: " << threshold << std::endl;
    //
    std::signal(SIGALRM, onAlarm);
    alarm(GRANULE);
}

void pipeError(int sig) {
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

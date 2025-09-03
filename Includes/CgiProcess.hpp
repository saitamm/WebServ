#ifndef CGIPROCESS_HPP
#define CGIPROCESS_HPP

#include <string>
#include <sstream>
#include <fstream>
#include <ctime>

class CgiProcess {
public:
    int clientFd;
    int pid;
    int pipeFd;
    bool finished;
    time_t start;
    string scriptPath;
    stringstream output;

    fstream inputStream; 

    CgiProcess(int fd, const string &script)
        : clientFd(fd), pid(-1), pipeFd(-1), start(0), scriptPath(script) {}

    ~CgiProcess() {
        if (inputStream.is_open()) inputStream.close();
    }

private:
    CgiProcess(const CgiProcess &);
    CgiProcess &operator=(const CgiProcess &);
};

#endif

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <string>
#include <unistd.h>
#include <netinet/in.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>

# define LIMIT 1024
using namespace std;
enum File
{
    File,
    STR,
};
class Request
{
public:
    Request();
    ~Request();
    string getMethod(void) const;
    string getHost(void) const;
    string getPort(void) const;
    string getPath(void) const;
    int getVServer(void) const;
    size_t getContentLength(void) const;
    void setMethod(string method);
    void setHost(string Host);
    void setPort(string Port);
    void setPath(string Path);
    // void setVServer(int val);
    void ParseRequest(int clientSocket);
    void setContentLength(size_t content);

private:
    string _method;
    string _host;
    string _port;
    string _path;
    size_t _ContentLength;
    enum File flag;
    string _body;

    map<string, string> _head;
};
// void ParseRequest(int clientSocket, Request &request);
#endif
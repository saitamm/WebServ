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
#include "Configfile.hpp"
#include <algorithm>
#include <cctype>

#define LIMIT 1024

class BadRequestException : public std::exception
{
public:
    const char *what() const throw()
    {
        return ("Bad Request\n");
    }
};

using namespace std;
class Request
{
public:
    Request();
    ~Request();
    string &getMethod(void);
    string &getHost(void);
    string &getUri(void);
    string &getQuery(void);
    string &getCtype(void);
    unsigned long long &getContentLength(void);
    void setMethod(string method);
    void setHost(string Host);
    // void setPort(string Port);
    // void setPath(string Path);
    // void setVServer(int val);
    void ParseRequest(int clientSocket);
    void setContentLength(unsigned long long content);

private:
    string _method;
    string _host;
    vector<string> _url;
    unsigned long long _ContentLength;
    string _body;

    map<string, string> _head;
};
void trim(string &str, string tr);
// void ParseRequest(int clientSocket, Request &request);
#endif
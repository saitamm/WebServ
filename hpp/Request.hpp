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
#include "ConfigFile.hpp"
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
class SocketErrorException : public std::exception
{
public:
    const char *what() const throw()
    {
        return ("Bad Socket\n");
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
    string &getFilename(void);
    unsigned long long &getContentLength(void);
    void ParseRequest(int clientSocket, ConfigFile &serv);
    ConfigFile &getConfigFile(void);

private:
    string _method;
    string _host;
    vector<string> _url;
    unsigned long long _ContentLength;
    string _body;
    string filename;
    map<string, string> _head;
    ConfigFile _serv;
    Location *_locat;
};
void trim(string &str, string tr);
// void ParseRequest(int clientSocket, Request &request);
#endif
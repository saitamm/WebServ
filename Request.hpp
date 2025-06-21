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

#define LIMIT 1024

class ErrorHttpVersionException : public std::exception
{
public:
    const char *what() const throw()
    {
        return ("Wrong HTTP Version\n");
    }
};
class HostIssueException : public std::exception
{
public:
    const char *what() const throw()
    {
        return ("Host issue\n");
    }
};
class MethodNotAllowedException : public std::exception
{
public:
    const char *what() const throw()
    {
        return ("Unsupported method\n");
    }
};
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
    int getPort(void) const;
    // string getPath(void) const;
    // int getVServer(void) const;
    size_t getContentLength(void) const;
    void setMethod(string method);
    void setHost(string Host);
    // void setPort(string Port);
    // void setPath(string Path);
    // void setVServer(int val);
    void ParseRequest(int clientSocket);
    void setContentLength(size_t content);

private:
    string _method;
    string _host;
    int  _port;
    vector<string> _url;
    size_t _ContentLength;
    // enum File flag;
    string _body;

    map<string, string> _head;
    // ConfigFile Vserv;
};
void trim(string &str, string tr);
// void ParseRequest(int clientSocket, Request &request);
#endif
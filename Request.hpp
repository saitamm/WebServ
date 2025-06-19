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
using namespace std;

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
    void setVServer(int val);
    void setContentLength(size_t content);

private:
    string _method;
    string _host;
    string _port;
    string _path;
    int _Vserver;
    size_t ContentLength;
};
void ParseRequest(string buffer, Request &request);
#endif
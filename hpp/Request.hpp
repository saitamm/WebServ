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
    Location *getLocation(void);
    unsigned long long &getContentLength(void);
    void ParseRequest(int clientSocket, ConfigFile &serv);
    ConfigFile &getConfigFile(void);
    string &getHeadvalue(string key);
    bool getfinishedHead(){return (_HeadF);};
    bool getfinishedBody(){return (_BodyF);};

private:
    string _method;
    string _host;
    vector<string> _url;
    bool _HeadF;
    bool _BodyF;
    unsigned long long _ContentLength;
    string _body;
    string filename;
    map<string, string> _head;
    ConfigFile _serv;
    Location *_locat;
};
void trim(string &str, string tr);
#endif
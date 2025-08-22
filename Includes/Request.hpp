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
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

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
    Location *getLocation(void);
    unsigned long long &getContentLength(void);
    ConfigFile &getConfigFile(void);
    string &getHeadvalue(string key);
    void ParseHeader(string &Header);
    bool getRedirectionStatus(void) const;
    void setRedirectionStatus(void);
    string &getrestHeader(void);

    // setters  
    void setMethod(const string &method);
    void setHost(const string &host);
    void setUrl(vector<string> &url);
    void setHeadvalue(const string &key, const string &value);
    void setLocation(Location *locat);
    void setHeader(string &key, string &value);
    void setRestHeader(const string &rest);
    void setConfigFile(ConfigFile &serv);

private:
    string _method;
    string _host;
    vector<string> _url;
    unsigned long long _ContentLength;
    map<string, string> _head;
    ConfigFile _serv;
    Location *_locat;
    string restHeader;
    bool _redir;
    string _boundary;
    string _contentType;
};
void trim(string &str, string tr);
Location *matchLocation(const std::string &uri, const std::vector<Location> &locations);
void split(string str, char c, vector<string> &resul);
void RedirectionRequest(Request &req);
#endif
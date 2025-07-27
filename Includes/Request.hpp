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
    string &getFilename(void);
    Location *getLocation(void);
    unsigned long long &getContentLength(void);
    ConfigFile &getConfigFile(void);
    string &getHeadvalue(string key);
    // void ParseHttpRequest(string &Header, int clientSocket, ConfigFile &serv, fstream &body);
    void ParseHeader(string &Header);
    int ParseBody(fstream &body, int clientSocket);
    bool getRedirectionStatus(void) const;
    void setRedirectionStatus(void);

    //setters
    void setMethod(const string &method);
    void setHost(const string &host);
    void setUrl(vector<string> &url);
    void setHeadvalue(const string &key, const string &value);
    void setLocation(Location *locat);
    void setHeader(string &key, string &value);
    void setRestHeader(const string &rest);
    size_t getTotalReceived(void) const { return totalReceived; }
    void setTotalReceived(size_t received) { totalReceived = received; }
    void setConfigFile(ConfigFile &serv) { _serv = serv; }

private:
    string _method;
    string _host;
    vector<string> _url;
    unsigned long long _ContentLength;
    // size_t totalReceived;
    map<string, string> _head;
    ConfigFile _serv;
    Location *_locat;
    size_t totalReceived;
    string restHeader;
    bool _redir;
};
void trim(string &str, string tr);
Location *matchLocation(const std::string &uri, const std::vector<Location> &locations);
void split(string str, char c, vector<string> &resul);
void RedirectionRequest(Request &req);
#endif
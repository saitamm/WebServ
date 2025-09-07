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
#include <stdlib.h>
#include "Exception.hpp"

#define LIMIT 1024


using namespace std;
class Request
{
public:
    Request();
    ~Request();

    //getters
    string &getMethod(void);
    string &getHost(void);
    string &getUri(void);
    string getQuery(void);
    string &getCtype(void);
    Location *getLocation(void);
    unsigned long long &getContentLength(void);
    ConfigFile &getConfigFile(void);
    string &getHeadvalue(string key);
    void ParseHeader(string &Header);
    bool getRedirectionStatus(void) const;
    void setRedirectionStatus(void);
    string &getrestHeader(void);
    int getPort(void);
    string &getCookie(void);

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
    string _port;
    vector<string> _url;
    unsigned long long _ContentLength;
    map<string, string> _head;
    ConfigFile _serv;
    Location *_locat;
    string restHeader;
    bool _redir;
    string _cookie;
};
void trim(string &str, string tr);
Location *matchLocation(const string &uri, const vector<Location> &locations);
void split(string str, char c, vector<string> &resul);
void RedirectionRequest(Request &req);
#endif
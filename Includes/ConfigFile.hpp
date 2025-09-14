#ifndef CONFIGFILE_HPP
#define CONFIGFILE_HPP

#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <string>
#include <bits/stdc++.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <memory>
#include<sys/types.h>
#include<sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include<set>

using namespace std;

enum Context
{
    NONE,
    SERVER,
    LOCATION
};

class Location
{
private:
    string path;
    set<string> methods;
    string auto_idx;
    string up_store;
    set<string> cgi_pass;
    set<string> cgi_extensions;
    string loc_idx;
    map<int, string> retur;
    string root_loc;

public:
    Location();
    ~Location();
    Location(const Location& other);
    Location& operator=(const Location& other);
    const string &getPath() const;
    void setPath(const string &p);
    const set<string> &getMethods() const;
    void add_method(const string &func);
    const string &getAuto_idx() const;
    void setAuto_idx(const string &index);
    const string &getUp_store() const;
    void setUp_store(const string &path);
    const set<string> &getCgi_pass() const;
    void setCgi_pass(const string &path);
    const set<string> &getCgi_ext() const;
    void setCgi_ext(const string &ext);
    const string &getLoc_idx() const;
    void setLoc_idx(const string &idx);
    const map<int, string> &getRetur() const;
    string getReturnTarget() const ;
    void add_retur(int err, string path);
    const string &getRoot_loc() const;
    void setRoot_loc(const string &r);
};

class ConfigFile
{
private:
    string name;
    string host;
    int port;
    string root;
    string index;
    map<int, string> error_page;
    size_t max_size;
    vector<Location> locations;
    static map<int, string> default_errors;

public:
    ConfigFile();
    ~ConfigFile();
    ConfigFile(const ConfigFile& other);
    ConfigFile& operator=(const ConfigFile& other);
    const string &getName() const;
    void setName(const string &n);
    const string &getHost() const;
    int setHost(const string &localhost);
    int getPort() const;
    int setPort(const string &p);
    const string &getRoot() const;
    void setRoot(const string &r);
    const string &getIndex() const;
    void setIndex(const string &idx);
    size_t getMax_size(void) const;
    int setMax_size(const string &size);
    const map<int, string> &getError_page() const;
    void add_error(int err, string path);
     vector<Location> &getLocations() ;
    void add_locations(const Location &loc);
    auto_ptr<vector<ConfigFile> > ParseConfigFile(string confFile);
    string &getDefaultErrorPage(int error) const;
    void initDefaultError(void);
};

class ErrorConfigFileException : public exception
{
public:
    const char *what() const throw()
    {
        return "Error, ConfigFile Is Incorrect!";
    }
};

class DuplicateDirectionException : public exception
{
public:
    const char *what() const throw()
    {
        return "Error, Duplicate Direction";
    }
};

class DuplicateMethodsException : public exception
{
public:
    const char *what() const throw()
    {
        return "Error, Duplicate Methods";
    }
};

class InvalidMethodException : public exception
{
public:
    const char *what() const throw()
    {
        return "Error, Invalid Method!";
    }
};

class InvalidErrorPageException : public exception
{
public:
    const char *what() const throw()
    {
        return "Error, Invalid Error page!";
    }
};

class DuplicateLocationException : public exception
{
public:
    const char *what() const throw()
    {
        return "Error, Duplicate Location Path !";
    }
};

class DuplicateServerException : public exception
{
public:
    const char *what() const throw()
    {
        return "Error, Duplicate Server !";
    }
};

class RedirectLoopException : public exception
{
public:
    const char *what() const throw()
    {
        return "Error, Redirect loop detected !";
    }
};

string trimLine(const string &line);
void ParseServer(string &key, string &value, ConfigFile &curr_server, string &new_line, map<string, bool> &alreadySeen);
void ParseLocation(string &key, string &value, string &new_line, Location &curr_loc, map<string, bool> &alreadySeen);
void CheckDupLoc(ConfigFile &curr_server, Location &curr_loc);
void CheckDupServ(auto_ptr<vector<ConfigFile> > &servers, ConfigFile &curr_server);
#endif

#ifndef CONFIGFILE_HPP
#define CONFIGFILE_HPP

#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <string>
#include <bits/stdc++.h>

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
        string cgi_pass;
        string loc_idx;
    public:
        const string& getPath() const;
        void setPath(const string& p);
        const set<string>& getMethods() const;
        void add_method(const string& func);
        const string& getAuto_idx() const;
        void setAuto_idx(const string& index);
        const string& getUp_store() const;
        void setUp_store(const string& path);
        const string& getCgi_pass() const;
        void setCgi_pass(const string& path);
        const string& getLoc_idx() const;
        void setLoc_idx(const string& idx);
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
        const string& getName() const;
        void setName(const string& n);
        const string& getHost() const;
        int setHost(const string& localhost);
        int getPort() const;
        int setPort(const string& p);
        const string& getRoot() const;
        void setRoot(const string& r);
        const string& getIndex() const;
        void setIndex(const string& idx);
        size_t getMax_size(void) const;
        int setMax_size(const string& size);
        const map<int, string>& getError_page() const;
        void add_error(int err, string path);
        const vector<Location>& getLocations() const;
        void add_locations(const Location& loc);
        vector<ConfigFile>* ParseConfigFile(string confFile);


        /// i add this to get the default error pages
        string getDefaultErrorPage(int error) const
        {
            return (default_errors.at(error));
        }
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

class DuplicateMethodsException : public exception{
    public:
        const char* what() const throw(){
            return "Error, Duplicate Methods";
        }
};

class InvalidErrorPageException : public exception{
    public:
        const char* what() const throw(){
            return "Error, Invalid Error page!";
        }
};

class DuplicateLocationException : public exception{
    public:
        const char* what() const throw(){
            return "Error, Duplicate Location Path !";
        }
};

class DuplicateServerException : public exception{
    public:
        const char* what() const throw(){
            return "Error, Duplicate Server !";
        }
};
string trimLine(const string &line);
void ParseServer(string &key, string &value, ConfigFile &curr_server, string &new_line, map<string, bool> &alreadySeen);
void ParseLocation(string &key, string &value, string &new_line, Location &curr_loc, map<string, bool> &alreadySeen);
void CheckDupLoc(ConfigFile& curr_server, Location& curr_loc);
void CheckDupServ(vector<ConfigFile> *servers, ConfigFile& curr_server);
#endif

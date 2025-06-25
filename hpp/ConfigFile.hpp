#ifndef CONFIGFILE_HPP
#define CONFIGFILE_HPP

#include<iostream>
#include<map>
#include<vector>
#include<fstream>
#include <string>
#include <bits/stdc++.h>

using namespace std;

enum Context {NONE, SERVER, LOCATION};

class Location
{  
    private:
        string path;
        vector<string> methods;
        string auto_idx;
        string up_store;
        string cgi_pass;
    public:
        string getPath() const;
        void setPath(const string& p);
        vector<string> getMethods() const;
        void add_method(const string& func);
        string getAuto_idx() const;
        void setAuto_idx(const string& index);
        string getUp_store() const;
        void setUp_store(const string& path);
        string getCgi_pass() const;
        void setCgi_pass(const string& path);
};

class ConfigFile
{
    private:
        string name;
        string host;
        string port;
        string root;
        string index;
        map<int, string> error_page;
        int max_size;
        vector<Location> locations;
    public:
        string getName() const;
        void setName(const string& n);
        string getHost() const;
        int setHost(const string& localhost);
        string getPort() const;
        int setPort(const string& p);
        string getRoot() const;
        void setRoot(const string& r);
        string getIndex() const;
        void setIndex(const string& idx);
        int getMax_size() const;
        int setMax_size(const string& size);
        map<int, string> getError_page() const;
        void add_error(int err, string path);
        vector<Location> getLocations() const;
        void add_locations(const Location& loc);
        int ParseConfigFile(string confFile);
        string trim_line(const string& line);
};

#endif

#include "../hpp/ConfigFile.hpp"

Location::Location(){}
Location::Location(const Location &loc)
{
    this->path = loc.path;
    this->methods = loc.methods;
    this->auto_idx = loc.auto_idx;
    this->up_store = loc.up_store;
    this->cgi_pass = loc.cgi_pass;
}

string ConfigFile::getName() const
{
    return name;
}

void ConfigFile::setName(const string& n)
{
    name = n;
}

string ConfigFile::getHost() const
{
    return host;
}

int ConfigFile::setHost(const string& localhost)
{
    int cpt = 0;
    stringstream ss(localhost);
    string bloc;
    while(!ss.eof())
    {
        getline(ss, bloc, '.');
        stringstream b(bloc);
        if (bloc.empty())
            return 0;
        for (size_t i = 0; i < bloc.size(); i++)
        {
            if (!isdigit(bloc[i]))
                return 0;
        }
        int nb;
        b >> nb;
        if (nb< 0 || nb > 255)
            return 0;
        cpt++;
    }
    if (cpt != 4)
        return 0;
    host = localhost;
    return 1;
}

string ConfigFile::getPort() const
{
    return port;
}

int ConfigFile::setPort(const string& p)
{
    for(size_t i = 0; i < p.size(); i++)
    {
        if (!isdigit(p[i]))
            return 0;
    }
    stringstream pp(p);
    int iport;
    pp >> iport;
    if (iport < 1 || iport > 65535)
        return 0;
    port = p;
    return 1;  
}

string ConfigFile::getRoot() const
{
    return root;
}

void ConfigFile::setRoot(const string& r)
{
    root = r;
}

string ConfigFile::getIndex() const
{
    return index;
}

void ConfigFile::setIndex(const string& idx)
{
    index = idx;
}

int ConfigFile::getMax_size() const
{
    return max_size;
}

int ConfigFile::setMax_size(const string& size)
{
    char unit = size[size.size() - 1];
    string s = size;
    if (unit == 'K' || unit == 'M' || unit == 'G')
        s = size.substr(0, size.size() - 1);
    else
        return 0;
    if(s.empty())
        return 0;
    for(size_t i = 0; i < s.size(); i++)
    {
        if (!isdigit(s[i]))
            return 0;
    }
    int sz;
    stringstream ss(s);
    ss >> sz;
    if (unit == 'K')
        sz*= 1024;
    else if(unit == 'M')
        sz*= 1024 * 1024;
        else if (unit == 'G')
        sz *= 1024 * 1024 * 1024; 
    max_size = sz;
    return 1;
}

map<int, string> ConfigFile::getError_page() const
{
    return error_page;
}

void ConfigFile::add_error(int err, string path)
{
    error_page[err] = path;
}

vector<Location> ConfigFile::getLocations() const
{
    return locations;
}

void ConfigFile::add_locations(const Location& loc)
{
    locations.push_back(loc);
}

vector<string> Location::getMethods() const
{
    return methods;
}

void Location::add_method(const string& func)
{
    methods.push_back(func);
}

string Location::getAuto_idx() const
{
    return auto_idx;
}

void Location::setAuto_idx(const string& index)
{
    auto_idx = index;
}

string Location::getUp_store() const
{
    return up_store;
}

void Location::setUp_store(const string& path)
{
    up_store = path;
}

string Location::getCgi_pass() const
{
    return cgi_pass;
}

void Location::setCgi_pass(const string& path)
{
    cgi_pass = path;
}

string Location::getPath() const
{
    return path;
}

void Location::setPath(const string& p)
{
    path = p;
}

#include "../../Includes/ConfigFile.hpp"

map<int, string> ConfigFile::default_errors;
void ConfigFile::initDefaultError(void)
{
    default_errors[200] = "srcs/Server/errors/200.html";
    default_errors[301] = "srcs/Server/errors/301.html";
    default_errors[400] = "srcs/Server/errors/400.html";
    default_errors[403] = "srcs/Server/errors/403.html";
    default_errors[404] = "srcs/Server/errors/404.html";
    default_errors[405] = "srcs/Server/errors/405.html";
    default_errors[409] = "srcs/Server/errors/409.html";
    default_errors[411] = "srcs/Server/errors/411.html";
    default_errors[500] = "srcs/Server/errors/500.html";
    default_errors[413] = "srcs/Server/errors/413.html";
    default_errors[431] = "srcs/Server/errors/431.html";
    default_errors[501] = "srcs/Server/errors/501.html";
    default_errors[502] = "srcs/Server/errors/502.html";
    default_errors[504] = "srcs/Server/errors/504.html";
    default_errors[413] = "srcs/Server/errors/413.html";
    default_errors[201] = "srcs/Server/errors/201.html";
}

ConfigFile::ConfigFile() {}

ConfigFile::~ConfigFile() {}

Location::Location() {}

Location::~Location() {}

Location::Location(const Location &other)
    : path(other.path), methods(other.methods), auto_idx(other.auto_idx),
      up_store(other.up_store), cgi_pass(other.cgi_pass),
      cgi_extensions(other.cgi_extensions), loc_idx(other.loc_idx), retur(other.retur), root_loc(other.root_loc) {}

Location &Location::operator=(const Location &other)
{
    if (this != &other)
    {
        path = other.path;
        methods = other.methods;
        auto_idx = other.auto_idx;
        up_store = other.up_store;
        cgi_pass = other.cgi_pass;
        cgi_extensions = other.cgi_extensions;
        loc_idx = other.loc_idx;
        retur = other.retur;
        root_loc = other.root_loc;
    }
    return *this;
}

ConfigFile::ConfigFile(const ConfigFile &other)
    : name(other.name), host(other.host), port(other.port),
      root(other.root), index(other.index),
      error_page(other.error_page), max_size(other.max_size),
      locations(other.locations) {}

ConfigFile &ConfigFile::operator=(const ConfigFile &other)
{
    if (this != &other)
    {
        name = other.name;
        host = other.host;
        port = other.port;
        root = other.root;
        index = other.index;
        error_page = other.error_page;
        max_size = other.max_size;
        locations = other.locations;
    }
    return *this;
}

const string &ConfigFile::getName() const
{
    return name;
}

void ConfigFile::setName(const string &n)
{
    name = n;
}

const string &ConfigFile::getHost() const
{
    return host;
}

int ConfigFile::setHost(const string &localhost)
{
    int cpt = 0;
    stringstream ss(localhost);
    string bloc;
    while (!ss.eof())
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
        if (nb < 0 || nb > 255)
            return 0;
        cpt++;
    }
    if (cpt != 4)
        return 0;
    host = localhost;
    return 1;
}

int ConfigFile::getPort() const
{
    return port;
}

int ConfigFile::setPort(const string &p)
{
    for (size_t i = 0; i < p.size(); i++)
    {
        if (!isdigit(p[i]))
            return 0;
    }
    stringstream pp(p);
    int iport;
    pp >> iport;
    if (iport < 1 || iport > 65535)
        return 0;
    port = iport;
    return 1;
}

const string &ConfigFile::getRoot() const
{
    return root;
}

void ConfigFile::setRoot(const string &r)
{
    if (r.empty())
        throw ErrorConfigFileException();
    size_t res = r.find(" ");
    if (res != string::npos)
        throw ErrorConfigFileException();
    root = r;
}

const string &ConfigFile::getIndex() const
{
    return index;
}

void ConfigFile::setIndex(const string &idx)
{
    index = idx;
}

size_t ConfigFile::getMax_size(void) const
{
    return max_size;
}

int ConfigFile::setMax_size(const string &size)
{
    char unit = size[size.size() - 1];
    string s = size;
    if (unit == 'K' || unit == 'M' || unit == 'G')
        s = size.substr(0, size.size() - 1);
    else
        return 0;
    if (s.empty())
        return 0;
    for (size_t i = 0; i < s.size(); i++)
    {
        if (!isdigit(s[i]))
            return 0;
    }
    size_t sz;
    stringstream ss(s);
    ss >> sz;
    if (unit == 'K')
        sz *= 1024;
    else if (unit == 'M')
        sz *= 1024 * 1024;
    else if (unit == 'G')
        sz *= 1024 * 1024 * 1024;
    max_size = sz;
    return 1;
}

const map<int, string> &ConfigFile::getError_page() const
{
    return error_page;
}

void ConfigFile::add_error(int err, string path)
{
    if (err < 200 || err >= 600)
        throw InvalidErrorPageException();
    if (path.empty())
        throw ErrorConfigFileException();
    size_t res = path.find(" ");
    if (path.size() < 5)
        throw InvalidErrorPageException();
    string file = path.substr(path.size() - 5);
    if (file != ".html" || res != string::npos)
    {
        throw InvalidErrorPageException();
    }
    error_page[err] = path;
}

 vector<Location> &ConfigFile::getLocations() 
{
    return locations;
}

void ConfigFile::add_locations(const Location &loc)
{
    locations.push_back(loc);
}

const set<string> &Location::getMethods() const
{
    return methods;
}

void Location::add_method(const string &func)
{
    if (func != "GET" && func != "POST" && func != "DELETE")
        throw InvalidMethodException();
    methods.insert(func);
}

const string &Location::getAuto_idx() const
{
    return auto_idx;
}

void Location::setAuto_idx(const string &index)
{
    if (index != "on" && index != "off")
        throw ErrorConfigFileException();
    auto_idx = index;
}

const string &Location::getUp_store() const
{
    return up_store;
}

void Location::setUp_store(const string &path)
{
    up_store = path;
}

const set<string> &Location::getCgi_pass() const
{
    return cgi_pass;
}

void Location::setCgi_ext(const string &ext)
{
    if (ext == ".py" || ext == ".php" || ext == ".pl")
        cgi_extensions.insert(ext);
    else
    {
        throw ErrorConfigFileException();
    }
}

const set<string> &Location::getCgi_ext() const
{
    return cgi_extensions;
}

void Location::setCgi_pass(const string &path)
{
    if (path == "/usr/bin/python3" || path == "/usr/bin/php" || path == "/usr/bin/perl")
        cgi_pass.insert(path);
    else
    {
        throw ErrorConfigFileException();
    }
}

const string &Location::getPath() const
{
    return path;
}

void Location::setPath(const string &p)
{
    path = p;
}

const string &Location::getLoc_idx() const
{
    return loc_idx;
}

void Location::setLoc_idx(const string &idx)
{
    size_t res = idx.find(" ");
    if (res != string::npos)
        throw ErrorConfigFileException();
    loc_idx = idx;
}

string &ConfigFile::getDefaultErrorPage(int error) const
{
    return (default_errors[error]);
}

const map<int, string> &Location::getRetur() const
{
    return retur;
}

string Location::getReturnTarget() const {
    if (retur.empty())
        return "";
    return retur.begin()->second;
}

void Location::add_retur(int err, string path)
{
    if (err < 200 || err >= 600)
        throw InvalidErrorPageException();
    if (!path.empty())
        retur[err] = path;
}

const string & Location::getRoot_loc() const
{
    return root_loc;
}

void Location::setRoot_loc(const string &r)
{
    if (r.empty())
        throw ErrorConfigFileException();
    size_t res = r.find(" ");
    if (res != string::npos)
        throw ErrorConfigFileException();
    root_loc = r;
}


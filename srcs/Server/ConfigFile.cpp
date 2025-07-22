#include "../../Includes/ConfigFile.hpp"

map<int, string> ConfigFile::default_errors;
void ConfigFile::initDefaultError(void)
{
    default_errors[200] = "srcs/Server/errors/200.html";
    default_errors[204] = "srcs/Server/errors/204.html";
    default_errors[301] = "srcs/Server/errors/301.html";
    default_errors[400] = "srcs/Server/errors/400.html";
    default_errors[403] = "srcs/Server/errors/403.html";
    default_errors[404] = "srcs/Server/errors/404.html";
    default_errors[405] = "srcs/Server/errors/405.html";
    default_errors[409] = "srcs/Server/errors/409.html";
    default_errors[500] = "srcs/Server/errors/500.html";
}

ConfigFile::ConfigFile() {}

ConfigFile::~ConfigFile() {}

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
    root = r;
}

const string &ConfigFile::getIndex() const
{
    return index;
}

void ConfigFile::setIndex(const string &idx)
{
    string file = idx.substr(idx.size() - 5, idx.size() - 1);
    if (file != ".html")
        throw ErrorConfigFileException();
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
    string file = path.substr(path.size() - 5, path.size() - 1);
    if (file != ".html")
        throw InvalidErrorPageException();
    error_page[err] = path;
}

const vector<Location> &ConfigFile::getLocations() const
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

const string &Location::getCgi_pass() const
{
    return cgi_pass;
}

void Location::setCgi_pass(const string &path)
{
    cgi_pass = path;
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
    string file = idx.substr(idx.size() - 5, idx.size() - 1);
    if (file != ".html")
        throw ErrorConfigFileException();
    loc_idx = idx;
}

string &ConfigFile::getDefaultErrorPage(int error) const
{
    return (default_errors[error]);
}

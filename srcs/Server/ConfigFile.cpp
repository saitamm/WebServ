#include "../../Includes/ConfigFile.hpp"
// ConfigFile::default_errors[200] = "errors/200.html";
// ConfigFile::default_errors[204] = "errors/204.html";
// ConfigFile::default_errors[301] = "errors/301.html";
// ConfigFile::default_errors[400] = "errors/400.html";
// ConfigFile::default_errors[403] = "errors/403.html";
// ConfigFile::default_errors[404] = "errors/404.html";
// ConfigFile::default_errors[405] = "errors/405.html";
// ConfigFile::default_errors[409] = "errors/409.html";
// ConfigFile::default_errors[500] = "errors/500.html";
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

size_t ConfigFile::getMax_size() const
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

vector<ConfigFile> *ConfigFile::ParseConfigFile(string confFile)
{
    string line, key, value;
    string new_line;
    Context bloc = NONE;
    Location curr_loc;
    ConfigFile curr_server;
    int is_location = 0, is_server = 0;
    vector<ConfigFile> *servers = new vector<ConfigFile>();
    map<string, bool> SeenInServer;
    map<string, bool> SeenInLocation;

    fstream file(confFile.c_str());
    if (!file.is_open())
        throw ErrorConfigFileException();
    while (getline(file, line))
    {
        new_line = trimLine(line);
        if (new_line.empty() || new_line[0] == '#')
            continue;
        if (new_line == "server")
        {
            is_server = 1;
            if (bloc == LOCATION)
            {
                CheckDupLoc(curr_server, curr_loc);
                curr_server.locations.push_back(curr_loc);
                bloc = SERVER;
            }
            if (bloc == SERVER)
            {
                if (!SeenInServer["listen"] || !SeenInServer["root"])
                {
                    cout << "Error is here!" << endl;
                    throw ErrorConfigFileException();
                }
                SeenInServer.clear();
                CheckDupServ(servers, curr_server);
                servers->push_back(curr_server);
            }
            curr_server = ConfigFile();
            bloc = SERVER;
            continue;
        }
        else if (new_line.substr(0, 8) == "location")
        {
            SeenInLocation.clear();
            is_location = 1;
            if (bloc == LOCATION)
            {
                CheckDupLoc(curr_server, curr_loc);
                curr_server.locations.push_back(curr_loc);
            }
            bloc = LOCATION;
            curr_loc = Location();
            string path = new_line.substr(new_line.find(" "));
            curr_loc.setPath(trimLine(path));
        }
        else if (bloc == SERVER)
            ParseServer(key, value, curr_server, new_line, SeenInServer);
        else if (bloc == LOCATION)
            ParseLocation(key, value, new_line, curr_loc, SeenInLocation);
    }
    if (!is_server || !is_location)
        throw ErrorConfigFileException();
    if (bloc == LOCATION)
    {
        CheckDupLoc(curr_server, curr_loc);
        curr_server.locations.push_back(curr_loc);
    }
    if (!SeenInServer["listen"] || !SeenInServer["root"])
        throw ErrorConfigFileException();
    CheckDupServ(servers, curr_server);
    servers->push_back(curr_server);

    //     for (size_t i = 0; i < servers->size(); ++i)
    // {
    //     cout << "===== Server " << i + 1 << " =====" << endl;
    //     cout << "Host: " << servers[i].getHost() << endl;
    //     cout << "Port: " << servers[i].getPort() << endl;
    //     cout << "Server Name: " << servers[i].getName() << endl;
    //     cout << "Root: " << servers[i].getRoot() << endl;
    //     cout << "Index: " << servers[i].getIndex() << endl;
    //     cout << "Max Body Size: " << servers[i].getMax_size() << endl;

    //     for (size_t i = 0; i < servers->size(); ++i)
    // {
    //     cout << "===== Server " << i + 1 << " =====" << endl;
    //     cout << "Host: " << servers[i].getHost() << endl;
    //     cout << "Port: " << servers[i].getPort() << endl;
    //     cout << "Server Name: " << servers[i].getName() << endl;
    //     cout << "Root: " << servers[i].getRoot() << endl;
    //     cout << "Index: " << servers[i].getIndex() << endl;
    //     cout << "Max Body Size: " << servers[i].getMax_size() << endl;

    //     // Print locations
    //     vector<Location> locs = servers[i].getLocations();
    //     for (size_t j = 0; j < locs.size(); ++j)
    //     {
    //         cout << "--- Location " << j + 1 << " ---" << endl;
    //         cout << "Path: " << locs[j].getPath() << endl;
    //         cout << "AutoIndex: " << locs[j].getAuto_idx() << endl;
    //         cout << "Upload Store: " << locs[j].getUp_store() << endl;
    //         cout << "CGI Pass: " << locs[j].getCgi_pass() << endl;

    //     // Print locations
    //     vector<Location> locs = servers[i].getLocations();
    //     for (size_t j = 0; j < locs.size(); ++j)
    //     {
    //         cout << "--- Location " << j + 1 << " ---" << endl;
    //         cout << "Path: " << locs[j].getPath() << endl;
    //         cout << "AutoIndex: " << locs[j].getAuto_idx() << endl;
    //         cout << "Upload Store: " << locs[j].getUp_store() << endl;
    //         cout << "CGI Pass: " << locs[j].getCgi_pass() << endl;

    //         vector<string> methods = locs[j].getMethods();
    //         cout << "Methods: ";
    //         for (size_t k = 0; k < methods.size(); ++k)
    //             cout << methods[k] << " ";
    //         cout << endl;
    //     }
    // }
    return servers;
}

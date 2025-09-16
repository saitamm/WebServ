#include "../../Includes/ConfigFile.hpp"

string trimLine(const string &line)
{
    size_t start = line.find_first_not_of(" \t\n\r\f\v");
    if (start == string::npos)
        return "";
    size_t end = line.find_last_not_of(" \t\n\r\f\v");
    return line.substr(start, end - start + 1);
}

void ParseServer(string &key, string &value, ConfigFile &curr_server, string &new_line, map<string, bool> &alreadySeen)
{
    stringstream ss(new_line);
    ss >> key;
    getline(ss, value);
    value = trimLine(value);
    if (alreadySeen[key] == true)
    {
        if (key == "error_page")
        {
            stringstream ss(value);
            int err;
            ss >> err;
            if (curr_server.getError_page().find(err) != curr_server.getError_page().end())
                throw DuplicateDirectionException();
        }
        else
            throw DuplicateDirectionException();
    }
    alreadySeen[key] = true;
    if (key == "listen")
    {
        size_t colon = value.find(":");
        if (!(curr_server.setHost(value.substr(0, colon))))
            throw ErrorConfigFileException();
        if (!curr_server.setPort(value.substr(colon + 1)))
            throw ErrorConfigFileException();
    }
    else if (key == "server_name")
        curr_server.setName(value);
    else if (key == "root")
        curr_server.setRoot(value);
    else if (key == "index")
        curr_server.setIndex(value);
    else if (key == "error_page")
    {
        stringstream err(value);
        int error;
        string path;
        err >> error;
        err >> path;
        curr_server.add_error(error, path);
    }
    else if (key == "client_max_body_size")
    {
        if (!curr_server.setMax_size(value))
            throw ErrorConfigFileException();
    }
    value = "";
}

void ParseLocation(string &key, string &value, string &new_line, Location &curr_loc, map<string, bool> &alreadySeen)
{
    stringstream ss(new_line);
    ss >> key;
    getline(ss, value);
    value = trimLine(value);
    if (alreadySeen[key] == true)
        throw DuplicateDirectionException();
    alreadySeen[key] = true;
    if (key == "methods")
    {
        stringstream ss(value);
        string method;
        while (ss >> method)
        {
            set<string> methods = curr_loc.getMethods();
            if (methods.find(method) != methods.end())
                throw DuplicateMethodsException();
            curr_loc.add_method(method);
        }
    }
    else if (key == "autoindex")
        curr_loc.setAuto_idx(value);
    else if (key == "index")
        curr_loc.setLoc_idx(value);
    else if (key == "upload_store")
        curr_loc.setUp_store(value);
    else if (key == "cgi_extensions")
    {
        stringstream ss(value);
        string ext;
        while (ss >> ext)
        {
            set<string> pass = curr_loc.getCgi_ext();
            curr_loc.setCgi_ext(ext);
        }
    }
    else if (key == "cgi_pass")
    {
        stringstream ss(value);
        string cgi;
        while (ss >> cgi)
        {
            set<string> pass = curr_loc.getCgi_pass();
            curr_loc.setCgi_pass(cgi);
        }
    }
    else if (key == "return")
    {
        stringstream ss(value);
        int err;
        string path;
        ss >> err;
        getline(ss, path);
        if (!path.empty() && path[0] == ' ')
            path.erase(0, 1);
        curr_loc.add_retur(err, path);
    }
    else if (key == "root")
        curr_loc.setRoot_loc(value);
}

void CheckDupLoc(ConfigFile &curr_server, Location &curr_loc)
{
    for (size_t i = 0; i < curr_server.getLocations().size(); i++)
    {
        if (curr_server.getLocations()[i].getPath() == curr_loc.getPath())
            throw DuplicateLocationException();
    }
}
void CheckDupServ(auto_ptr<vector<ConfigFile> > &servers, ConfigFile &curr_server)
{
    for (size_t i = 0; i < servers->size(); i++)
    {
        if (servers->at(i).getHost() == curr_server.getHost() && servers->at(i).getPort() == curr_server.getPort())
            throw DuplicateServerException();
        if (servers->at(i).getName() == curr_server.getName() && servers->at(i).getPort() == curr_server.getPort() && servers->at(i).getHost() == curr_server.getHost())
            throw DuplicateServerException();
    }
}

void checkRedir(Location &curr_loc)
{
    std::map<std::string, std::string> redirects;
    const Location &loc = curr_loc;
    if (!loc.getRetur().empty())
    {
        string from = loc.getPath();
        string to = loc.getReturnTarget();

        if (from == to)
            throw RedirectLoopException();

        redirects[from] = to;
    }
}

void checkReturnLoop(ConfigFile &curr_server)
{
    vector<Location> locations = curr_server.getLocations();
    for (size_t i = 0; i < locations.size(); i++)
    {
        if (locations[i].getRetur().empty())
            continue;
        string start = locations[i].getPath();          // /
        string target = locations[i].getReturnTarget(); // /images
        string current = target;
        size_t depth = 0;
        if (!current.empty() && current[0] == '/')
        {
            if (depth++ > locations.size())
                throw RedirectLoopException();
            bool found = false;
            for (size_t j = 0; j < locations.size(); j++)
            {
                if (i != j)
                {
                    if (locations[j].getPath() == current)
                    {
                        if (locations[j].getReturnTarget().empty())
                        {
                            current = "";
                            break;
                        }
                        current = locations[j].getReturnTarget();
                        if (current == start)
                            throw RedirectLoopException();
                        found = true;
                    }
                }
            }
            if (!found)
                break;
        }
    }
}

auto_ptr<vector<ConfigFile> > ConfigFile::ParseConfigFile(string confFile)
{
    string line, key, value;
    string new_line;
    Context bloc = NONE;
    Location curr_loc;
    ConfigFile curr_server;
    int is_location = 0, is_server = 0;
    auto_ptr<vector<ConfigFile> > servers(new vector<ConfigFile>());
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
                    throw ErrorConfigFileException();

                SeenInServer.clear();
                CheckDupServ(servers, curr_server);
                servers->push_back(curr_server);
                checkReturnLoop(curr_server);
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
                checkRedir(curr_loc);
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
        checkRedir(curr_loc);
    }

    if (!SeenInServer["listen"] || !SeenInServer["root"])
        throw ErrorConfigFileException();

    CheckDupServ(servers, curr_server);
    servers->push_back(curr_server);
    checkReturnLoop(curr_server);
    return servers;
}

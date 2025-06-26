#include "../hpp/ConfigFile.hpp"

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
        throw DuplicateDirectionException();
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
        size_t spc = value.find(" ");
        stringstream err(value.substr(0, spc));
        int error;
        err >> error;
        curr_server.add_error(error, value.substr(spc + 1));
    }
    else if (key == "client_max_body_size")
    {
        if (!curr_server.setMax_size(value))
            throw ErrorConfigFileException();
    }
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
        size_t del = value.find(" ");
        if (del != string::npos)
        {
            curr_loc.add_method(value.substr(0, del));
            curr_loc.add_method(value.substr(del + 1));
        }
        else
            curr_loc.add_method(value);
    }
    else if (key == "autoindex")
        curr_loc.setAuto_idx(value);
    else if (key == "upload_store")
    {
        curr_loc.setUp_store(value);
    }
    else if (key == "cgi_pass")
        curr_loc.setCgi_pass(value);
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
    map<string, bool> alreadySeen;

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
            alreadySeen.clear();
            is_server = 1;
            if (bloc == LOCATION)
            {
                curr_server.locations.push_back(curr_loc);
                bloc = SERVER;
            }
            if (bloc == SERVER)
            {
                servers->push_back(curr_server);
            }
            curr_server = ConfigFile();
            bloc = SERVER;
            continue;
        }
        else if (new_line.substr(0, 8) == "location")
        {
            alreadySeen.clear();
            is_location = 1;
            if (bloc == LOCATION)
                curr_server.locations.push_back(curr_loc);
            bloc = LOCATION;
            curr_loc = Location();
            string path = new_line.substr(new_line.find(" "));
            curr_loc.setPath(trimLine(path));
        }
        else if (bloc == SERVER)
            ParseServer(key, value, curr_server, new_line, alreadySeen);
        else if (bloc == LOCATION)
            ParseLocation(key, value, new_line, curr_loc, alreadySeen);
    }
    if (!is_server || !is_location)
        throw ErrorConfigFileException();
    if (bloc == LOCATION)
        curr_server.locations.push_back(curr_loc);
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

    //     // Print error pages
    //     map<int, string> errs = servers[i].getError_page();
    //     for (map<int, string>::iterator it = errs.begin(); it != errs.end(); ++it)
    //     {
    //         cout << "Error Page [" << it->first << "] => " << it->second << endl;
    //     }

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

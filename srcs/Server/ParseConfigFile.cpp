#include "../../Includes/ConfigFile.hpp"

string trimLine(const string &line)
{
    size_t start = line.find_first_not_of(" \t\n\r\f\v");
    if (start == string::npos)
        return "";
    size_t end = line.find_last_not_of(" \t\n\r\f\v");
    return line.substr(start, end - start + 1);
}

void    ParseServer(string &key, string &value, ConfigFile &curr_server, string &new_line, map<string, bool> &alreadySeen)
{
    stringstream ss(new_line);
    ss >> key;
    getline(ss, value);
    value = trimLine(value);
    if(alreadySeen[key] == true)
    {
        if (key == "error_page")
        {
            stringstream ss(value);
            int err;
            ss >> err;
            if(curr_server.getError_page().find(err) != curr_server.getError_page().end())
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
        while(ss >> method)
        {
            set<string>methods = curr_loc.getMethods();
            if (methods.find(method) != methods.end())
                throw DuplicateMethodsException();                
            curr_loc.add_method(method);
        }
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

void CheckDupLoc(ConfigFile& curr_server, Location& curr_loc)
{
    for (size_t i = 0; i < curr_server.getLocations().size(); i++)
    {
        if (curr_server.getLocations()[i].getPath() == curr_loc.getPath())
            throw DuplicateLocationException(); 
    }
}
void CheckDupServ(vector<ConfigFile> *servers, ConfigFile& curr_server)
{
    for(size_t i = 0; i < servers->size(); i++)
    {
        if (servers->at(i).getName() == curr_server.getName() && servers->at(i).getPort() == curr_server.getPort())
            throw DuplicateServerException();
    }
}

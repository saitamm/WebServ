#include "../../../Includes/Response.hpp"

void getContentType(string &real_path, Response &resp)
{
    string five = real_path.substr(real_path.size() - 5, real_path.size() - 1);
    string four = real_path.substr(real_path.size() - 4, real_path.size() - 1);
    string three = real_path.substr(real_path.size() - 3, real_path.size() - 1);
    if (".html" == five)
    {
        resp.setType("text/html");
        return;
    }
    if ((".jpeg" == five || ".jpg" == four))
    {
        resp.setType("image/jpeg");
        return;
    }
    if (".png" == four)
    {
        resp.setType("image/png");
        return;
    }
    if (".css" == four)
    {
        resp.setType("text/css");
        return;
    }
    if (".js" == three)
    {
        resp.setType("application/javascript");
        return;
    }
    if (".mp4" == four)
    {
        resp.setType("video/mp4");
        return;
    }
    resp.setType("application/octet-stream");
}

void generateResponse(Response& resp, string& real_path)
{
    ifstream file(real_path.c_str(), ios::binary);
    if (!file.is_open())
    {
        setCodeStatus(resp, 403);
        return;
    }
    std::string buffer((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    resp.setStatus(200);
    resp.setBody(buffer);
    getContentType(real_path, resp);
}

void handleGet(Response &resp)
{
    string real_path = resp.getRequest()->getConfigFile().getRoot() + resp.getRequest()->getUri();
    struct stat path;
    if (stat(real_path.c_str(), &path) == -1)
    {
        setCodeStatus(resp, 404);
        return;
    }
    if (S_ISREG(path.st_mode))
    {
        generateResponse(resp, real_path);
    }
    else if (S_ISDIR(path.st_mode))
    {
        if (real_path[real_path.size() - 1] != '/')
            real_path += '/';
        if (resp.getRequest()->getLocation()->getLoc_idx().empty())
        {
            if (resp.getRequest()->getLocation()->getAuto_idx() != "on")
            {
                if (resp.getRequest()->getConfigFile().getIndex().empty())
                {
                    return setCodeStatus(resp, 404);
                }
                else
                {
                    string indx_path = resp.getRequest()->getConfigFile().getIndex();
                    generateResponse(resp, indx_path);
                }
            }
            else
            {
                DIR* dir = opendir(real_path.c_str());
                if (dir != NULL)
                {
                    stringstream html;
                    html << "<html><body><h1>Listing directory /" << real_path << "</h1><ul>";
                    struct dirent* entry;
                    while((entry = readdir(dir)) != NULL)
                    {
                        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
                            continue;
                       html << "<li><a href='./" << real_path << entry->d_name << "'>" << entry->d_name << "</a></li>";

                    }
                    html << "</ul></body></html>";
                    string ss = html.str();
                    closedir(dir);
                    resp.setStatus(200);
                    resp.setBody(ss);
                    resp.setType("text/html");
                }
            }
        }
        else
        {
            string path_idx = resp.getRequest()->getLocation()->getLoc_idx();
            generateResponse(resp, path_idx);
        }
    }
    
}
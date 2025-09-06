#include "../../../Includes/Client.hpp"

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

void generateResponse(Response &resp, string &real_path)
{
    getContentType(real_path, resp);
    size_t size = getFileSize(real_path);
    if (size < 1024)
    {
        resp.setChunkFile(real_path);
        if (!resp.getChunkFile().is_open())
        {
            cerr << "Failed to open chunk file: " << real_path << endl;
            setCodeStatus(resp, 500);
            resp.setResponseStatus(Nonchunked);
            return;
        }
        resp.setStatus(200);
        stringstream buffer;
        buffer << resp.getChunkFile().rdbuf();
        resp.setBodyResp(buffer.str());
        resp.getChunkFile().close();
        return;
    }
    if (resp.getResponseStatus() == Nonchunked)
    {
        resp.setChunkFile(real_path);
        if (!resp.getChunkFile().is_open())
        {
            cerr << "Failed to open chunk file: " << real_path << endl;
            setCodeStatus(resp, 500);
            resp.setResponseStatus(Nonchunked);
            return;
        }
        resp.setStatus(200);
        resp.setResponseStatus(First);
        return;
    }
    resp.setStatus(200);
    char buffer[8192];
    resp.getChunkFile().read(buffer, sizeof(buffer));
    string line(buffer, resp.getChunkFile().gcount());
    resp.setBodyResp(resp.getRestSend() + line);
    resp.setResponseStatus(chunked);
    if (line.size() == 0 && resp.getRestSend().empty())
    {
        resp.setResponseStatus(Last);
        resp.getChunkFile().close();
        return;
    }
}



int handleGet(Response &resp, int clientFd, int epollFd, map<int, CgiProcess *> &cgis)
{
    string root = resp.getRequest()->getConfigFile().getRoot();
    string uri = resp.getRequest()->getUri();
    string real_path;
    string prefix = "/" + root;
    while (uri.rfind(prefix, 0) == 0)
    {
        uri.erase(0, prefix.size());
    }
    real_path = root + uri;
    struct stat path;
    if (stat(real_path.c_str(), &path) == -1)
    {
        setCodeStatus(resp, 404);
        return 0;
    }
    if (S_ISREG(path.st_mode))
    {
        string ext = getExt(resp);
        if(isCgi(ext) && resp.getRequest()->getLocation()->getCgi_pass().empty())
        {
            setCodeStatus(resp, 500);
            return 0;
        }
        else if (!resp.getRequest()->getLocation()->getCgi_pass().empty() && isCgiExtension(ext, resp))
        {
            checkCgiGet(resp, real_path, clientFd, epollFd, cgis, ext);
            return 1;
        }
        else
        {
            generateResponse(resp, real_path);
        }
    }
    else if (S_ISDIR(path.st_mode))
    {
        if (real_path[real_path.size() - 1] != '/')
            real_path += '/';
        string index = resp.getRequest()->getConfigFile().getRoot() + "/" + resp.getRequest()->getLocation()->getLoc_idx();
        if ((!resp.getRequest()->getLocation()->getLoc_idx().empty()) && (stat(index.c_str(), &path) != -1))
        {
            size_t dotPos = index.find_last_of('.');
            string ext = index.substr(dotPos);
            if (!resp.getRequest()->getLocation()->getCgi_pass().empty() && isCgiExtension(ext, resp))
            {
                checkCgiGet(resp, index, clientFd, epollFd, cgis, ext);
                return 1;
            }
            if (resp.getRequest()->getCookie().empty())
            {
                cout << ":::::::::::::::::::::;\n";
                string indx_path = resp.getRequest()->getLocation()->getLoc_idx();
                generateResponse(resp, indx_path);
                return (0);
            }
            else
            {
                string indx_path = "./index1.html";
                generateResponse(resp, indx_path);
                return (0);
            }
        }
        else
        {
            if (resp.getRequest()->getLocation()->getAuto_idx() == "on")
            {
                DIR *dir = opendir(real_path.c_str());
                if (dir != NULL)
                {
                    stringstream html;
                    html << "<html><body><h1>Listing directory /" << real_path << "</h1><ul>";
                    struct dirent *entry;
                    while ((entry = readdir(dir)) != NULL)
                    {
                        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
                            continue;
                        html << "<li><a href='./" << real_path << entry->d_name << "'>" << entry->d_name << "</a></li>";
                    }
                    html << "</ul></body></html>";
                    string ss = html.str();
                    closedir(dir);
                    resp.setStatus(200);
                    resp.setBodyResp(ss);
                    resp.setType("text/html");
                }
            }
            else
            {
                setCodeStatus(resp, 403);
                return 0;
            }
        }
    }
    return 0;
}

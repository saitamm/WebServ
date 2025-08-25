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

void generateResponse(Response &resp, string &real_path)
{
    getContentType(real_path, resp);
    size_t size = getFileSize(real_path);
    if (size < 5999999)
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
    char buffer[1024];
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

void checkCgi(Response &resp, string &real_path)
{
    int fd[2];
    if (pipe(fd) == -1)
    {
        setCodeStatus(resp, 500);
        return;
    }
    pid_t pid = fork();
    if (pid < 0)
    {
        setCodeStatus(resp, 500);
        return;
    }
    else if (pid == 0)
    {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        string arg = resp.getRequest()->getLocation()->getCgi_pass();
        char *argv[] = {strdup(arg.c_str()), strdup(real_path.c_str()), NULL};
        char *envp[] = {strdup("REQUEST_METHOD=GET"), strdup(("SCRIPT_FILENAME=" + real_path).c_str()), NULL};
        execve(arg.c_str(), argv, envp);
        exit(1);
    }
    else
    {
        close(fd[1]);
        char buffer[4096];
        stringstream output;
        ssize_t bytesRead;
        while ((bytesRead = read(fd[0], buffer, sizeof(buffer))) > 0)
        {
            output.write(buffer, bytesRead);
        }
        close(fd[0]);

        int status;
        waitpid(pid, &status, 0);

        resp.setStatus(200);
        string outStr = output.str();
        resp.setBodyResp(outStr);
        resp.setType("text/html");
    }
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
        if (!resp.getRequest()->getLocation()->getCgi_pass().empty())
        {
            cout << "here ---->" << resp.getRequest()->getLocation()->getCgi_pass() << endl;
            checkCgi(resp, real_path);
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
                    if (resp.getRequest()->getCookie().empty())
                    {
                        string indx_path = resp.getRequest()->getConfigFile().getIndex();
                        generateResponse(resp, indx_path);
                    }
                    else
                    {
                        string indx_path = "./index1.html";
                        generateResponse(resp, indx_path);
                    }
                }
            }
            else
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
        }
        else
        {
            string path_idx = resp.getRequest()->getLocation()->getLoc_idx();
            generateResponse(resp, path_idx);
        }
    }
}
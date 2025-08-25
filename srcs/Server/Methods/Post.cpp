#include "../../../Includes/Client.hpp"

int SupportUpload(Response &resp)
{
    if (resp.getRequest()->getLocation()->getUp_store().empty())
        return (1);
    return (0);
}

unsigned int getSize(int clientSocket)
{
    string line;
    char c;
    while (recv(clientSocket, &c, 1, 0))
    {
        line += c;
        if (line.size() > 2 && line.substr(line.size() - 2) == "\r\n")
            break;
    }
    unsigned int BufferSize;
    stringstream ll(line);
    string l;
    ll >> l;
    std::stringstream ss;
    ss << std::hex << l;
    ss >> BufferSize;
    return (BufferSize);
}

void NonChunkedBody(Response &resp, int clientSocket)
{
    char buf[1024];
    ssize_t bytesRead = 0;
    if (!resp.getTotalReceived())
    {
        resp.setTotalReceived(resp.getRequest()->getrestHeader().size());
        resp.getFile().write(resp.getRequest()->getrestHeader().c_str(), resp.getTotalReceived());
    }
    string Body;
    bytesRead = recv(clientSocket, buf, sizeof(buf), 0);
    if (bytesRead <= 0)
        return;
    resp.getFile().write(buf, bytesRead);
    resp.getFile().flush();
    resp.setTotalReceived(bytesRead);
}

int ChunkedBody(Response &resp, int clientSocket)
{
    int bytesRead = 0;
    unsigned int BufferSize;
    if (!resp.getTotalReceived())
    {
        stringstream ll(resp.getRequest()->getrestHeader());
        string l;
        ll >> l;
        std::stringstream ss;
        ss << std::hex << l;
        ss >> BufferSize;
        string line;
        line = ll.str();
        line.erase(0, l.size() + 2);
        resp.setTotalReceived(5);
        int size;
        while (1)
        {
            size = (BufferSize > line.size()) ? line.size() : BufferSize;
            resp.getFile().write(line.substr(0, size).c_str(), size);
            line.erase(0, size);
            if (line.empty())
                break;
            line.erase(0, 2);
            stringstream kk(line);
            string k;
            kk >> k;
            stringstream jj;
            jj << hex << k;
            jj >> BufferSize;
            if (BufferSize == 0)
                return (1);
            line.erase(0, k.size() + 2);
        }
        if (size < (int)BufferSize)
        {
            char buf[BufferSize - size];
            bytesRead = recv(clientSocket, buf, sizeof(buf), 0);
            if (bytesRead == 0)
            {
                throw BadRequestException();
            }
            resp.getFile().write(buf, bytesRead);
            resp.getFile().flush();
        }
    }
    else
    {
        if (resp.getReceived() == 0)
        {
            resp.setBufferSize(getSize(clientSocket));
            if (resp.getBufferSize() == 0)
                return (1);
        }
        char buff[1024];
        size_t read = min(resp.getBufferSize() - resp.getReceived(), (unsigned int)sizeof(buff));
        bytesRead = recv(clientSocket, buff, read, 0);
        if (bytesRead <= 0)
        {
            throw BadRequestException();
        }
        resp.getFile().write(buff, bytesRead);
        resp.getFile().flush();
        resp.setReceived(bytesRead);
        if (resp.getReceived() == resp.getBufferSize())
        {
            resp.restartChunk();
            char del[2];
            bytesRead = recv(clientSocket, del, sizeof(del), 0);
            if (bytesRead <= 0)
                throw SocketErrorException();
        }
    }
    return (0);
}

map<string, string> CgiEnv(Response &resp, string &path)
{
    map<string, string> env;
    env["REQUEST_METHOD"] = "POST";
    env["SCRIPT_FILENAME"] = path;
    // try
    // {
    //     env["QUERY_STRING"] = resp.getRequest()->getQuery();
    // }
    // catch (exception &e)
    // {
    //     cout << e.what() << endl;
    // }
    env["SERVER_NAME"] = resp.getRequest()->getConfigFile().getName();
    std::stringstream ss;
    ss << resp.getRequest()->getConfigFile().getPort();
    env["SERVER_PORT"] = ss.str();
    env["CONTENT_TYPE"] = resp.getContenttype();
    std::stringstream sss;
    sss << resp.getRequest()->getContentLength();
    env["CONTENT_LENGTH"] = sss.str();
    return env;
}

void executeCgi(Response& resp, int clientFd, int epollFd, map<int, CgiProcess*> &cgis)
{
    string path;
    path = resp.getFileName();
    map<string, string> env = CgiEnv(resp, path);
    int fd_in[2];
    int fd_out[2];
    if (pipe(fd_in) == -1)
    {
        setCodeStatus(resp, 500);
        return;
    }
    if (pipe(fd_out) == -1)
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
        dup2(fd_in[0], 0);
        dup2(fd_out[1], 1);
        close(fd_in[1]);
        close(fd_out[0]);
        vector<char *> envp;
        for (map<string, string>::iterator it = env.begin(); it != env.end(); ++it)
        {
            std::string entry = it->first + "=" + it->second;
            envp.push_back(strdup(entry.c_str()));
        }
        envp.push_back(NULL);
        string cgiPath = checkCgiPath(resp);
        char *argv[] = {strdup(cgiPath.c_str()), (char *)path.c_str(), NULL};
        execve(cgiPath.c_str(), argv, envp.data());
        perror("execve failed");
        exit(1);
    }
    else if (pid > 0)
    {
        close(fd_in[0]);
        close(fd_out[1]);
        resp.getFile().seekg(0, std::ios::beg);
        char buf[1024];
        while (resp.getFile().read(buf, sizeof(buf)) || resp.getFile().gcount() > 0)
        {
            cerr << "**************************************\n";
            write(1, buf, resp.getFile().gcount());
            write(fd_in[1], buf, resp.getFile().gcount());
        }
        close(fd_in[1]);
        setNonBlocking(fd_out[0]);
        epoll_event ev;
        memset(&ev, 0, sizeof(ev));
        ev.data.fd = fd_out[0];
        ev.events = EPOLLIN | EPOLLHUP;
        epoll_ctl(epollFd, EPOLL_CTL_ADD, fd_out[0], &ev);
        CgiProcess *proc = new CgiProcess(clientFd, path);
        proc->pid = pid;
        proc->pipeFd = fd_out[0];
        proc->start = time(NULL);
        cgis[fd_out[0]] = proc;
    }
}

    // void executeCgi(Response& resp, int clientFd, int epollFd, map<int, CgiProcess*> &cgis)
    // {
    //     int fd[2];
    //    // string path = resp.getRequest()->getConfigFile().getRoot() + resp.getRequest()->getUri();
    //    string path = resp.getFileName(); 
    //    if (pipe(fd) == -1)
    //     {
    //         setCodeStatus(resp, 500);
    //         return;
    //     }
    //     pid_t pid = fork();
    //     if (pid < 0)
    //     {
    //         setCodeStatus(resp, 500);
    //         return;
    //     }
    //     if (pid == 0)
    //     {
    //         dup2(fd[1], STDOUT_FILENO);
    //         close(fd[1]);
    //         close(fd[0]);
    //         string cgiPath = checkCgiPath(resp);
    //         map<string, string>env = CgiEnv(resp, path);
    //         vector<char *> envp;
    //         for (map<string, string>::iterator it = env.begin(); it != env.end(); ++it)
    //         {
    //             std::string entry = it->first + "=" + it->second;
    //             envp.push_back(strdup(entry.c_str()));
    //         }
    //         envp.push_back(NULL);
    //         char *argv[] = {strdup(cgiPath.c_str()), (char *)path.c_str(), NULL};
    //         execve(cgiPath.c_str(), argv, envp.data());
    //         perror("execve failed");
    //         exit(1);
    //     }
    //     else
    //     {
    //         close(fd[1]);
    //         setNonBlocking(fd[0]);
    //         epoll_event ev;
    //         memset(&ev, 0, sizeof(ev));
    //         ev.data.fd = fd[0];
    //         ev.events = EPOLLIN | EPOLLHUP;
    //         epoll_ctl(epollFd, EPOLL_CTL_ADD, fd[0], &ev);
    //         CgiProcess *proc = new CgiProcess(clientFd, path);
    //         proc->pid = pid;
    //         proc->pipeFd = fd[0];
    //         proc->start = time(NULL);
    //         cgis[fd[0]] = proc;
    //     }
    // }

    int handlePost(Response & resp, int clientSocket, int epollFd, map<int, CgiProcess *> &cgis)
    {
        if (SupportUpload(resp))
        {
            setCodeStatus(resp, 403);
            return (1);
        }
        if (!resp.getRequest()->getLocation()->getCgi_pass().empty())
        {
            if (resp.getRequest()->getHeadvalue("Transfer-Encoding").empty())
            {
                NonChunkedBody(resp, clientSocket);
                if (resp.getTotalReceived() == resp.getRequest()->getContentLength())
                {
                    executeCgi(resp, clientSocket, epollFd, cgis);
                    return (2);
                }
            }
            else
            {
                if (ChunkedBody(resp, clientSocket))
                {
                    executeCgi(resp, clientSocket, epollFd, cgis);
                    return (2);
                }
            }
            return (0);
        }
        if (resp.getRequest()->getHeadvalue("Transfer-Encoding").empty())
        {
            NonChunkedBody(resp, clientSocket);
            if (resp.getTotalReceived() == resp.getRequest()->getContentLength())
            {
                if (resp.getFile().is_open())
                    resp.getFile().close();
                setCodeStatus(resp, 200);
                return (1);
            }
        }
        else
        {
            if (ChunkedBody(resp, clientSocket))
            {
                if (resp.getFile().is_open())
                    resp.getFile().close();
                setCodeStatus(resp, 200);
                return (1);
            }
        }
        return (0);
    }
#include "../../Includes/Client.hpp"

bool isCgiExtension(const string &ext, Response &resp)
{
    set<string> extensions = resp.getRequest()->getLocation()->getCgi_ext();
    if (extensions.count(ext))
        return 1;
    return 0;
}

string getExt(Response &resp)
{
    string path = resp.getRequest()->getUri();
    size_t dotPos = path.find_last_of('.');
    if (dotPos == string::npos)
        return "";
    string ext = path.substr(dotPos);
    return ext;
}

string checkCgiPath(Response &resp, string &ext)
{
    const set<string> cgi = resp.getRequest()->getLocation()->getCgi_pass();
    if (ext == ".py")
    {
        if (cgi.count("/usr/bin/python3"))
            return "/usr/bin/python3";
    }
    else if (ext == ".sh")
    {
        if (cgi.count("/usr/bin/bash"))
            return "/usr/bin/bash";
    }
    else if (ext == ".php")
    {
        if (cgi.count("/usr/bin/php"))
            return "/usr/bin/php";
    }
    else if (ext == ".pl")
    {
        if (cgi.count("/usr/bin/perl"))
            return "/usr/bin/perl";
    }
    return "";
}

map<string, string> CgiEnv(Response &resp)
{
    map<string, string> env;
    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["REQUEST_METHOD"] = "POST";
    env["SCRIPT_FILENAME"] = resp.getRequest()->getConfigFile().getRoot() + resp.getRequest()->getUri();
    env["QUERY_STRING"] = resp.getRequest()->getQuery();
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

void checkCgiGet(Response &resp, string &real_path, int clientFd, int epollFd, map<int, CgiProcess *> &cgis, string &ext)
{
    string arg = checkCgiPath(resp, ext);
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ " << arg << endl;
    map<string, string> env = CgiEnv(resp);
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
        char *argv[] = {strdup(arg.c_str()), strdup(real_path.c_str()), NULL};
        vector<char *> envp;
        for (map<string, string>::iterator it = env.begin(); it != env.end(); ++it)
        {
            std::string entry = it->first + "=" + it->second;
            envp.push_back(strdup(entry.c_str()));
        }
        envp.push_back(NULL);
        execve(arg.c_str(), argv, envp.data());
        perror("execve failed");
        exit(1);
    }
    else
    {
        close(fd[1]);
        setNonBlocking(fd[0]);
        epoll_event ev;
        memset(&ev, 0, sizeof(ev));
        ev.data.fd = fd[0];
        ev.events = EPOLLIN | EPOLLHUP;
        if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd[0], &ev) == -1)
        {
            perror("epoll_ctl: add");
            return;
        }
        CgiProcess *proc = new CgiProcess(clientFd, real_path);
        proc->pid = pid;
        proc->pipeFd = fd[0];
        proc->start = time(NULL);

        cgis[fd[0]] = proc;
    }
}

void checkCgiPost(Response &resp, int clientFd, int epollFd, map<int, CgiProcess *> &cgis, string &ext)
{
    string path;
    path = resp.getFileName();
    map<string, string> env = CgiEnv(resp);
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
        string cgiPath = checkCgiPath(resp, ext);
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
            write(fd_in[1], buf, resp.getFile().gcount());
        }
        close(fd_in[1]);
        setNonBlocking(fd_out[0]);
        epoll_event ev;
        memset(&ev, 0, sizeof(ev));
        ev.data.fd = fd_out[0];
        ev.events = EPOLLIN | EPOLLHUP;
        if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd_out[0], &ev) == -1)
        {
            perror("epoll_ctl: add");
            return;
        }
        CgiProcess *proc = new CgiProcess(clientFd, path);
        proc->pid = pid;
        proc->pipeFd = fd_out[0];
        proc->start = time(NULL);
        cgis[fd_out[0]] = proc;
    }
}

void sendCleanUp(Response &resp, int epollFd, CgiProcess *proc, std::map<int, Client *> &clients, std::map<int, CgiProcess *> &cgis)
{
    if (epoll_ctl(epollFd, EPOLL_CTL_DEL, proc->pipeFd, NULL) == -1)
    {
        perror("epoll_ctl: add");
        return;
    }
    close(proc->pipeFd);

    std::map<int, Client *>::iterator it = clients.find(proc->clientFd);
    if (it != clients.end())
    {
        Client *client = it->second;
        if (client->getEvent().events & EPOLLOUT)
        {
            SendResponse(resp, proc->clientFd);
        }
        client->setStatus(Finished);
        close(proc->clientFd);

        delete client;
        clients.erase(proc->clientFd);
    }
    cgis.erase(proc->pipeFd);
    delete proc;
}

void CgiEvent(int fd, int epollFd, map<int, Client *> &clients, map<int, CgiProcess *> &cgis)
{
    CgiProcess *proc = cgis[fd];
    char buffer[1024];
    ssize_t bytesRead = read(proc->pipeFd, buffer, sizeof(buffer));
    if (bytesRead > 0)
        proc->output.write(buffer, bytesRead);

    int status;
    pid_t result = waitpid(proc->pid, &status, WNOHANG);

    if (result == proc->pid || difftime(time(NULL), proc->start) > 5)
    {
        Client *client = clients[proc->clientFd];
        Response &resp = *client->getResp();
        resp.setRequest(*client->getRequest());

        if (result == proc->pid)
        {
            if (WIFEXITED(status))
            {
                int exitCode = WEXITSTATUS(status);
                if (exitCode == 0)
                {
                    std::string outStr = proc->output.str();
                    if (resp.getRequest()->getMethod() == "POST")
                    {
                        resp.getFile().close();
                        resp.getFile().open(resp.getFileName().c_str(), ios::out | ios::trunc | ios::binary);
                        resp.getFile().write(outStr.data(), outStr.size());
                        resp.getFile().flush();
                        if (resp.getFile().is_open())
                            resp.getFile().close();
                        setCodeStatus(resp, 200);
                    }
                    else
                    {
                        resp.setBodyResp(outStr);
                        resp.setStatus(200);
                        resp.setType("text/html");
                    }
                }
                else
                {
                    std::cerr << "CGI exited with error code " << exitCode << std::endl;
                    setCodeStatus(resp, 500);
                }
            }
            else if (WIFSIGNALED(status))
            {
                int sig = WTERMSIG(status);
                std::cerr << "CGI killed by signal " << sig << std::endl;
                setCodeStatus(resp, 500);
            }
        }
        else
        {
            std::cerr << "[CGI] Timeout, killing pid=" << proc->pid << std::endl;
            kill(proc->pid, SIGKILL);
            waitpid(proc->pid, NULL, 0);
            setCodeStatus(resp, 500);
        }
        sendCleanUp(resp, epollFd, proc, clients, cgis);
    }
}
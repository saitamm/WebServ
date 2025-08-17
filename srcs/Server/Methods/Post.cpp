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
                throw BadRequestException();
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
            throw BadRequestException();
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
    try
    {
        env["QUERY_STRING"] = resp.getRequest()->getQuery();
    }
    catch (exception &e)
    {
        cout << e.what() << endl;
    }
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

void executeCgi(Response &resp)
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
        string cgiPath = resp.getRequest()->getLocation()->getCgi_pass();
        char *argv[] = {strdup(cgiPath.c_str()), (char *)path.c_str(), NULL};
        execve(cgiPath.c_str(), argv, envp.data());
        perror("execve failed");
        exit(1);
    }
    else if (pid > 0)
    {
        int timeout = 13; // seconds
        time_t start = time(NULL);
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
        char buffer[1024];
        string cgiOutput;
        while (true) {
            ssize_t bytesRead = read(fd_out[0], buffer, sizeof(buffer));
            if (bytesRead > 0) {
                cgiOutput.append(buffer, bytesRead);
            }

            // check timeout
            if (time(NULL) - start > timeout) {
                cerr << "[CGI] Timeout, killing process\n";
                kill(pid, SIGKILL);
                waitpid(pid, NULL, 0);
                setCodeStatus(resp, 500);
                break;
            }

            // check if child finished
            int status;
            pid_t result = waitpid(pid, &status, WNOHANG);
            if (result == pid) {
                // child exited
                break;
            }
        }
        close(fd_out[0]);
        resp.getFile().close();
        resp.getFile().open(resp.getFileName().c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
        resp.getFile().write(cgiOutput.data(), cgiOutput.size());
        resp.getFile().flush();
        setCodeStatus(resp, 200);
        if (resp.getFile().is_open())
            resp.getFile().close();
    }
}



int handlePost(Response &resp, int clientSocket)
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
                executeCgi(resp);
                return (1);
            }
        }
        else
        {
            if (ChunkedBody(resp, clientSocket))
            {
                executeCgi(resp);
                return (1);
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
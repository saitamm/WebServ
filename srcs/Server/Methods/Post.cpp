#include "../../../Includes/Response.hpp"

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
        throw SocketErrorException();
    cout << "Read " << bytesRead << " bytes\n";
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
        cout << "Read " << bytesRead << " bytes\n";
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
    map<string, string>env;
    env["REQUEST_METHOD"]   = "POST";
    env["SCRIPT_FILENAME"]  = path; 
    env["QUERY_STRING"]     = resp.getRequest()->getQuery();
    env["SERVER_NAME"]      = resp.getRequest()->getConfigFile().getName();
    std::stringstream ss;
    ss << resp.getRequest()->getConfigFile().getPort();
    env["SERVER_PORT"] = ss.str();
    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["PATH_INFO"]        = path; 
    env["REMOTE_ADDR"]      = env["REMOTE_ADDR"] = resp.getRequest()->getHeadvalue("X-Forwarded-For").empty() ? "127.0.0.1" : resp.getRequest()->getHeadvalue("X-Forwarded-For");
    env["CONTENT_TYPE"]   = resp.getContenttype();
    std::stringstream sss;
    sss << resp.getRequest()->getContentLength();
    env["CONTENT_LENGTH"] = sss.str();
    return env;
}

void executeCgi(Response &resp)
{
    string path = resp.getRequest()->getConfigFile().getRoot() + resp.getRequest()->getUri();
    map<string, string> env = CgiEnv(resp, path);

    int fd_in[2];
    int fd_out[2];
    if (pipe(fd_in) == -1 || pipe(fd_out) == -1)
        throw std::runtime_error("Failed to create pipes for CGI");

    pid_t pid = fork();
    if (pid < 0)
        throw std::runtime_error("Failed to fork process for CGI");

    if (pid == 0) 
    {
        dup2(fd_in[0], STDIN_FILENO);
        dup2(fd_out[1], STDOUT_FILENO);
        close(fd_in[1]);
        close(fd_out[0]);

        vector<char*> envp;
        for (map<string, string>::iterator it = env.begin(); it != env.end(); ++it)
        {
            string entry = it->first + "=" + it->second;
            envp.push_back(strdup(entry.c_str()));
        }
        envp.push_back(NULL);

        char *argv[] = { (char*)path.c_str(), NULL };
        execve(path.c_str(), argv, envp.data());
        perror("execve failed");
        exit(1);
    }
    else // parent
    {
        close(fd_in[0]);
        close(fd_out[1]);

        if (!resp.getBody().empty())
        {
            write(fd_in[1], resp.getBody().data(), resp.getBody().size());
        }
        else if (resp.getFile().is_open())
        {
            resp.getFile().seekg(0, ios::beg);
            string body((istreambuf_iterator<char>(resp.getFile())),
                            istreambuf_iterator<char>());
            write(fd_in[1], body.data(), body.size());
        }
        close(fd_in[1]);

        char buffer[4096];
        string cgiOutput;
        ssize_t bytesRead;
        while ((bytesRead = read(fd_out[0], buffer, sizeof(buffer))) > 0)
        {
            cgiOutput.append(buffer, bytesRead);
        }
        close(fd_out[0]);

        resp.setBodyResp(cgiOutput);
        cout << "************* >> "<< resp.getBody() << endl;
        setCodeStatus(resp, 200);
    }
}



int handlePost(Response &resp, int clientSocket)
{
    if (!resp.getRequest()->getLocation()->getCgi_pass().empty())
    {
        cout << "waaaaaaaaaaaaaaa3\n";
        if (resp.getRequest()->getHeadvalue("Transfer-Encoding").empty())
        {
            NonChunkedBody(resp, clientSocket);
            if (resp.getTotalReceived() == resp.getRequest()->getContentLength())
            {
                cout << "hereeeeeeeeeeeee\n";
                executeCgi(resp); 
                return (1);
            }
        }
        else
        {
            if (ChunkedBody(resp, clientSocket))
            {
                cout << "hereeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\n";
                executeCgi(resp); 
                return (1);
            }
        }
        return (0); 
    }
    if (SupportUpload(resp))
    {
        setCodeStatus(resp, 403);
        return (1);
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
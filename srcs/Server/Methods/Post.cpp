#include "../../../Includes/Client.hpp"

int SupportUpload(Response &resp)
{
    if (resp.getRequest()->getLocation()->getUp_store().empty())
        return (1);
    return (0);
}

unsigned int getSize(int clientSocket, Response &resp)
{
    char buffer[1024];
    int byteread;
    if ((byteread = recv(clientSocket, buffer, sizeof(buffer), 0)) <= 0)
        throw BadRequestException();
    string line(buffer, byteread);
    line.erase(0, 2);
    stringstream ll(line);
    unsigned int BufferSize;
    string l;
    ll >> l;
    stringstream ss;
    ss << hex << l;
    ss >> BufferSize;
    line.erase(0, line.find("\n") + 1);
    if (resp.getFile().write(line.c_str(), line.size()).fail())
        throw BadRequestException();
    resp.getFile().flush();
    resp.setReceived(line.size());
    return (BufferSize);
}

void NonChunkedBody(Response &resp, int clientSocket)
{
    char buf[1024];
    int bytesRead = 0;
    if (!resp.getTotalReceived())
    {
        resp.setTotalReceived(resp.getRequest()->getrestHeader().size());
        if (resp.getFile().write(resp.getRequest()->getrestHeader().c_str(), resp.getTotalReceived()).fail())
            throw BadRequestException();
        // return ;
    }
    if (resp.getRequest()->getContentLength() == 0)
        return;
    if (resp.getTotalReceived() != resp.getRequest()->getContentLength())
    {

        string Body;
        bytesRead = recv(clientSocket, buf, sizeof(buf), 0);
        if (bytesRead <= 0)
        {
            if (resp.getTotalReceived() < resp.getRequest()->getContentLength())
                throw BadRequestException();
            if (bytesRead == -1)
                resp.setResponseStatus(Finish);
            return;
        }

        if (resp.getFile().write(buf, bytesRead).fail())
            throw BadRequestException();
        resp.getFile().flush();
        resp.setTotalReceived(bytesRead);
    }
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
        stringstream ss;
        ss << hex << l;
        ss >> BufferSize;
        string line;
        line = ll.str();
        line.erase(0, l.size() + 2);
        int size;
        resp.setTotalReceived(5);
        size = (BufferSize > line.size()) ? line.size() : BufferSize;
        while (1)
        {
            size = (BufferSize > line.size()) ? line.size() : BufferSize;
            if (resp.getFile().write(line.substr(0, size).c_str(), size).fail())
                throw BadRequestException();
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
            if (bytesRead <= 0)
            {
                throw BadRequestException();
            }
            if (resp.getFile().write(buf, bytesRead).fail())
                throw BadRequestException();
            resp.getFile().flush();
        }
    }
    else
    {
        if (resp.getReceived() == 0)
        {
            resp.setBufferSize(getSize(clientSocket, resp));
            if (resp.getBufferSize() == 0)
            {
                return (1);
            }
            if (resp.getFile().write(resp.getRestPost().c_str(), resp.getRestPost().size()).fail())
                throw BadRequestException();
        }
        else
        {

            char buff[1024];
            size_t read = min(resp.getBufferSize() - resp.getReceived(), (unsigned int)sizeof(buff));
            bytesRead = recv(clientSocket, buff, read, 0);
            if (bytesRead <= 0)
                throw BadRequestException();
            if (resp.getFile().write(buff, bytesRead).fail())
                throw BadRequestException();
            resp.getFile().flush();
            resp.setReceived(bytesRead);
            if (resp.getReceived() == resp.getBufferSize())
            {
                resp.restartChunk();
            }
        }
    }
    return (0);
}

int handlePost(Response &resp, int clientSocket, int epollFd, map<int, CgiProcess *> &cgis)
{
    string ext = getExt(resp);
    if (!resp.getRequest()->getLocation()->getCgi_pass().empty() && isCgiExtension(ext, resp))
    {
        checkCgiPost(resp, clientSocket, epollFd, cgis, ext);
        return (2);
    }
    return (0);
}

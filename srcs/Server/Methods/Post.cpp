#include "../../../Includes/Client.hpp"

int SupportUpload(Response &resp)
{
    if (resp.getRequest()->getLocation().getUp_store().empty())
        return (1);
    return (0);
}

void CreatUploadFile(Response &resp)
{
    srand(time(0));
    stringstream ll;
    string type = resp.getRequest()->getHeadvalue("Content-Type").substr(resp.getRequest()->getHeadvalue("Content-Type").find('/') + 1);
    ll << rand();
    string f = ll.str() + "." + type;
    if (type.empty())
        throw BadRequestException();
    string root;
    if (resp.getRequest()->getLocation().getRoot_loc().empty())
        root = resp.getRequest()->getConfigFile().getRoot();
    else
        root = resp.getRequest()->getLocation().getRoot_loc();
    string store = root + "/" + resp.getRequest()->getLocation().getUp_store();
    string Up = store + "/" + f;
    resp.getFile().open(Up.c_str(), ios::out | ios::trunc | ios::binary);
    resp.setFileName(Up);
    // cout << "This is the upload file  = " << Up << endl;
    if (!resp.getFile().is_open())
    {
        struct stat st;
        if (stat(store.c_str(), &st) == -1)
            throw ServerErrorException();
        if (access(resp.getRequest()->getLocation().getUp_store().c_str(), W_OK) == -1)
            throw ForbiddenException();
        throw ServerErrorException();
    }
}

int ReadBody(Response &resp, int clientSocket)
{
    if (SupportUpload(resp))
        throw ForbiddenException();
    if (resp.getRequest()->getHeadvalue("Transfer-Encoding").empty())
    {
        NonChunkedBody(resp, clientSocket);
        if (resp.getTotalReceived() == resp.getRequest()->getContentLength())
        {
            if (resp.getFile().is_open())
                resp.getFile().close();
            setCodeStatus(resp, 200);
            return 1;
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
unsigned int getSize(int clientSocket, Response &resp)
{
    char buffer[1024];
    int byteread;
    if ((byteread = recv(clientSocket, buffer, sizeof(buffer), 0)) <= 0)
    {
        throw ConnectionFailedException();
        return (0);
    }
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
    size_t size = 0;
    while (1)
    {
        size = (BufferSize > line.size()) ? line.size() : BufferSize;
        resp.setTotalReceived(size);
        if (resp.getFile().write(line.c_str(), size).fail())
        {
            remove(resp.getFileName().c_str());
            throw ServerErrorException();
        }
        resp.getFile().flush();
        resp.setReceived(size);
        line.erase(0, size);
        stringstream ll(line);
        string l;
        ll >> l;
        stringstream ss;
        ss << hex << l;
        ss >> BufferSize;
        if (BufferSize == 0 || line.empty())
            break;
    }
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
        {
            remove(resp.getFileName().c_str());
            throw ServerErrorException();
        }
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
            {
                remove(resp.getFileName().c_str());
                throw BadRequestException();
            }
            if (bytesRead == -1)
            {
                remove(resp.getFileName().c_str());
                throw ConnectionFailedException();
            }
            return;
        }

        if (resp.getFile().write(buf, bytesRead).fail())
        {
            remove(resp.getFileName().c_str());
            throw ServerErrorException();
        }
        resp.getFile().flush();
        resp.setTotalReceived(bytesRead);
    }
}

int ChunkedBody(Response &resp, int clientSocket)
{
    if (!resp.getTotalReceived())
    {
        unsigned int BufferSize;
        stringstream ll(resp.getRequest()->getrestHeader());
        string l;
        ll >> l;
        stringstream ss;
        ss << hex << l;
        ss >> BufferSize;
        string line;
        line = ll.str();
        line.erase(0, l.size() + 2);
        unsigned int size;
        size = (BufferSize > line.size()) ? line.size() : BufferSize;
        if (resp.getFile().write(line.substr(0, size).c_str(), size).fail())
        {
            remove(resp.getFileName().c_str());
            throw ServerErrorException();
        }
        resp.setTotalReceived(size);
        resp.setBufferSize(BufferSize);
        cout << "when total receive is 0 ="<<BufferSize << endl;
        line.erase(0, size);
        line.erase(0, 2);
        resp.getRequest()->setRestHeader(line);
        if (BufferSize == size)
        {
            resp.restartChunk();
        }
        else
            resp.setReceived(size);
        cout << "after first =" << resp.getReceived() << endl;
    }
    if (!resp.getReceived())
    {
        if (resp.getRequest()->getrestHeader().empty())
        {
            cout << "header rest is empty \n";
            char buf[1024];
            int bytesRead = 0;
            bytesRead = recv(clientSocket, buf, sizeof(buf), 0);
            if (bytesRead <= 0)
            {
                remove(resp.getFileName().c_str());
                throw ConnectionFailedException();
            }
            string line(buf);
            // line.erase(0, 2);
            unsigned int BufferSize;
            stringstream kk(line);
            string k;
            kk >> k;
            stringstream jj;
            jj << hex << k;
            jj >> BufferSize;
            cout << "buffer chunk i read heeeeeere = "<<BufferSize << endl;
            if (BufferSize == 0)
            {
                return (1);
            }
            line.erase(0, k.size() + 2);
            unsigned int size;
            size = (BufferSize > line.size()) ? line.size() : BufferSize;
            cout << "i am what first read =" << size <<endl;
            if (resp.getFile().write(line.substr(0, size).c_str(), size).fail())
            {
                remove(resp.getFileName().c_str());
                throw ServerErrorException();
            }
            resp.setBufferSize(BufferSize);
            if (BufferSize == size)
            {
                line.erase(0, size);
                line.erase(0, 2);
                resp.setTotalReceived(size);
                resp.getRequest()->setRestHeader(line);
                resp.restartChunk();
            }
            else
            {
                resp.setReceived(size);
            }
        }
        else
        {
            unsigned int BufferSize;
            stringstream ll(resp.getRequest()->getrestHeader());
            string l;
            ll >> l;
            stringstream ss;
            ss << hex << l;
            ss >> BufferSize;
            string line;
            line = ll.str();
            line.erase(0, l.size() + 2);
            unsigned int size;
            cout << "---------" << resp.getRequest()->getrestHeader()  << "-----"<<endl;
            cout << BufferSize << endl;
            if (BufferSize == 0)
            {
                return (1);
            }
            size = (BufferSize > line.size()) ? line.size() : BufferSize;
            if (resp.getFile().write(line.substr(0, size).c_str(), size).fail())
            {
                remove(resp.getFileName().c_str());
                throw ServerErrorException();
            }
            line.erase(0, size);
            line.erase(0, 2);
            resp.setTotalReceived(size);
            resp.getRequest()->setRestHeader(line);
            resp.setBufferSize(BufferSize);
            if (BufferSize == size)
                resp.restartChunk();
            else
                resp.setReceived(size);
        }
    }
    else
    {
        cout << "----\n";
        char buf[resp.getBufferSize() - resp.getReceived()];
        int bytesRead = 0;
        bytesRead = recv(clientSocket, buf, sizeof(buf), 0);
        if (bytesRead <= 0)
        {
            remove(resp.getFileName().c_str());
            throw ConnectionFailedException();
        }
        cout << "last read from chunk " << bytesRead <<endl;
        string line(buf);
        if (resp.getFile().write(line.c_str(), line.size()).fail())
        {
            remove(resp.getFileName().c_str());
            throw ServerErrorException();
        }
        resp.setTotalReceived(bytesRead);
        resp.restartChunk();
    }
    return (0);
}

int handlePost(Response &resp, int clientSocket, int epollFd, map<int, CgiProcess *> &cgis)
{
    string ext = getExt(resp);
    if (!resp.getRequest()->getLocation().getCgi_pass().empty() && isCgiExtension(ext, resp))
    {
        checkCgiPost(resp, clientSocket, epollFd, cgis, ext);
        return (2);
    }
    return (0);
}

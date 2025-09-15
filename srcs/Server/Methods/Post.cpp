#include "../../../Includes/Client.hpp"

int SupportUpload(Response &resp)
{
    if (resp.getRequest()->getLocation().getUp_store().empty())
        return (1);
    return (0);
}
void GetType(string &type, string &ext)
{
    string arr[] = {"text/html", "text/plain", "image/png", "image/jpeg", "image/jpg", "video/mp4"};
    vector<string> types(arr, arr + sizeof(arr) / sizeof(arr[0]));
    if (find(types.begin(), types.end(), type) != types.end())
    {
        ext = type.substr(type.find('/') + 1);
        return;
    }
    ext = "bin";
}
void CreatUploadFile(Response &resp)
{
    srand(time(0));
    stringstream ll;
    string type = resp.getRequest()->getHeadvalue("Content-Type").substr(resp.getRequest()->getHeadvalue("Content-Type").find('/') + 1);
    ll << rand();
    GetType(resp.getRequest()->getHeadvalue("Content-Type"), type);
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
    NonChunkedBody(resp, clientSocket);
    if (resp.getTotalReceived() == resp.getRequest()->getContentLength())
    {
        if (resp.getFile().is_open())
            resp.getFile().close();
        setCodeStatus(resp, 201);
        return 1;
    }
    return (0);
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

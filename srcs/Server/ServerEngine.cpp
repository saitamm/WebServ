#include "../../Includes/Client.hpp"

void setCodeStatus(Response &resp, int error)
{
    if (resp.getRequest()->getRedirectionStatus())
    {
        resp.setStatus(resp.getRequest()->getLocation()->getRetur().begin()->first);
        return;
    }
    resp.setStatus(error);
    map<int, string> body = resp.getRequest()->getConfigFile().getError_page();
    if (body[error].empty())
    {
        string defaultErrorPage = resp.getRequest()->getConfigFile().getDefaultErrorPage(error);
        getContentType(defaultErrorPage, resp);
        ifstream file(defaultErrorPage.c_str());
        if (!file.is_open())
        {
            cerr << "❌ Failed to open file: " << body[error] << endl;
            return;
        }
        string buffer((istreambuf_iterator<char>(file)),
                      istreambuf_iterator<char>());
        resp.setBodyResp(buffer);
        file.close();
    }
    else
    {
        getContentType(body[error], resp);
        ifstream file(body[error].c_str());
        if (!file.is_open())
        {
            cerr << "❌ Failed to open file: " << body[error] << endl;
            return;
        }
        string buffer((istreambuf_iterator<char>(file)),
                      istreambuf_iterator<char>());
        resp.setBodyResp(buffer);
        file.close();
    }
}
void generateUser(Response &resp)
{
    if (resp.getRequest()->getCookie().empty())
    {
        string Id;
        stringstream ss;
        ss << rand();
        Id = ss.str();
        resp.setSessionId(Id);
    }
    else
    {
        resp.setSessionId(resp.getRequest()->getCookie());
    }
}
size_t getFileSize(const string &path)
{
    struct stat st;
    if (stat(path.c_str(), &st) == 0)
    {
        return st.st_size;
    }
    return 0;
}

void NonchunkedResponse(Response &resp, int clientSocket)
{
    stringstream response;
    response << "HTTP/1.1 " << resp.getStatus() << " " << resp.getStatusValue(resp.getStatus()) << "\r\n";
    response << "Content-Type: " << resp.getType() << "\r\n";
    if (resp.getRequest()->getRedirectionStatus())
    {
        Location *loc = resp.getRequest()->getLocation();
        if (loc)
        {
            const map<int, string> &returMap = loc->getRetur();
            if (!returMap.empty())
                response << "Location: " << returMap.begin()->second << "\r\n";
        }
    }
    generateUser(resp);
    response << "Set-Cookie: user=" << resp.getSessionId() << "\r\n";
    response << "Content-Length: " << resp.getBody().size() << "\r\n\r\n";

    response << resp.getBody();
    int bytesend;
    bytesend = send(clientSocket, response.str().c_str(), response.str().size(), MSG_NOSIGNAL);
    if (bytesend != (int)response.str().size() && bytesend != -1)
    {
        resp.setRestSend(response.str().substr(bytesend));
        return;
    }
    resp.setResponseStatus(Finish);
}

void chunkedResponse(Response &resp, int clientSocket)
{
    if (resp.getResponseStatus() == First)
    {
        stringstream response;
        response << "HTTP/1.1 " << resp.getStatus() << " " << resp.getStatusValue(resp.getStatus()) << "\r\n";
        response << "Content-type: " << resp.getType() << "\r\n";
        response << "Transfer-Encoding: chunked\r\n";
        generateUser(resp);
        response << "Set-Cookie: user=" << resp.getSessionId() << "\r\n";
        response << "Connection: close\r\n\r\n";
        int bytesend;
        bytesend = send(clientSocket, response.str().c_str(), response.str().size(), MSG_NOSIGNAL);
        if (bytesend != (int)response.str().size() && bytesend != -1)
            resp.setRestSend(response.str().substr(bytesend));
        resp.setResponseStatus(chunked);
    }
    else if (resp.getResponseStatus() == chunked)
    {
        stringstream response;
        response << hex << resp.getBody().size() << "\r\n";
        response << resp.getBody() << "\r\n";
        string responseStr = response.str();
        int bytesend;
        bytesend = send(clientSocket, responseStr.c_str(), responseStr.size(), MSG_NOSIGNAL);
        if (bytesend != (int)response.str().size() && bytesend != -1)
            resp.setRestSend(response.str().substr(bytesend));
    }
    else if (resp.getResponseStatus() == Last)
    {
        stringstream response;
        response << "0\r\n\r\n";
        int bytesend;
        bytesend = send(clientSocket, response.str().c_str(), response.str().size(), MSG_NOSIGNAL);
        resp.setResponseStatus(Finish);
    }
}
void SendResponse(Response &resp, int clientSocket)
{
    if (resp.getResponseStatus() == Nonchunked)
    {
        NonchunkedResponse(resp, clientSocket);
    }
    else
    {
        chunkedResponse(resp, clientSocket);
    }
}

int allowMethod(Location loc, string method)
{
    for (set<string>::const_iterator it = loc.getMethods().begin(); it != loc.getMethods().end(); ++it)
    {
        if (*it == method)
            return (1);
    }
    return (0);
}

void handleClientRequest(std::map<int, Client *> &clients, int clientSocket, std::auto_ptr<std::vector<ConfigFile> > &servers, int epollFd, std::map<int, CgiProcess *> &cgis)
{
    clients[clientSocket]->updateActivity();
    try
    {
        clients[clientSocket]->getResp()->initStatusCode();
        if (clients[clientSocket]->getEvent().events == (EPOLLIN))
            clients[clientSocket]->ParseHttpRequest(*clients[clientSocket], clientSocket, servers);
        if (clients[clientSocket]->getStatus() == Processing || clients[clientSocket]->getStatus() == Sending)
        {
            clients[clientSocket]->getEvent().events = EPOLLOUT;
            if (epoll_ctl(epollFd, EPOLL_CTL_MOD, clientSocket, &clients[clientSocket]->getEvent()) == -1)
            {
                perror("epoll_ctl: add");
                return;
            }
            clients[clientSocket]->buildResponse(clientSocket, epollFd, cgis);
        }
    }
    catch (const exception &e)
    {
        clients[clientSocket]->getEvent().events = EPOLLOUT;
        if (epoll_ctl(epollFd, EPOLL_CTL_MOD, clientSocket, &clients[clientSocket]->getEvent()) == -1)
        {
            perror("epoll_ctl: add");
            return;
        }
        clients[clientSocket]->getResp()->setRequest(*clients[clientSocket]->getRequest());
        clients[clientSocket]->setStatus(Sending);
        if (!clients[clientSocket]->getRequest()->getRedirectionStatus())
            setCodeStatus(*clients[clientSocket]->getResp(), 400);
        else
            setCodeStatus(*clients[clientSocket]->getResp(), 0);
    }

    if ((clients[clientSocket]->getEvent().events & EPOLLOUT) && (clients[clientSocket]->getStatus() == Sending))
    {
        SendResponse(*clients[clientSocket]->getResp(), clientSocket);
        clients[clientSocket]->setNewSessionId(clients[clientSocket]->getResp()->getSessionId());

        if (clients[clientSocket]->getResp()->getResponseStatus() == Finish)
            clients[clientSocket]->setStatus(Finished);
    }
    if (clients[clientSocket]->getStatus() == Finished)
    {
        cout << "WAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAa\n";
        if (epoll_ctl(epollFd, EPOLL_CTL_DEL, clientSocket, NULL) == -1)
        {
            perror("epoll_ctl: add");
            return;
        }
        close(clientSocket);
        delete clients[clientSocket];
        clients.erase(clientSocket);
    }
}

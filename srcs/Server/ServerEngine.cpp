#include "../../Includes/Client.hpp"

void ServerError(Response &resp)
{
    resp.setStatus(500);
    resp.setType("text/html");
    std::string body = "<!DOCTYPE html>\n"
                       "<html>\n"
                       "<head>\n"
                       "  <meta charset=\"UTF-8\">\n"
                       "  <title>500 Internal Server Error</title>\n"
                       "</head>\n"
                       "<body style=\"margin: 0; height: 100vh; display: flex; justify-content: center; align-items: center; background-color: white;\">\n"
                       "  <h1 style=\"color: red; font-size: 2.5em;\">500 Internal Server Error</h1>\n"
                       "</body>\n"
                       "</html>\n";
    resp.setBodyResp(body);
}
void setCodeStatus(Response &resp, int error)
{
    if (resp.getRequest()->getRedirectionStatus())
    {
        resp.setStatus(resp.getRequest()->getLocation().getRetur().begin()->first);
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
            ServerError(resp);
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
            ServerError(resp);
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
    if (resp.getRequest()->getRedirectionStatus())
    {
        if (!resp.getRequest()->getLocation().getRetur().empty())
        {
            response << "Location: " << resp.getRequest()->getLocation().getRetur().begin()->second << "\r\n";
        }
    }
    if (!resp.getRequest()->getRedirectionStatus())
    {
        response << "Content-Type: " << resp.getType() << "\r\n";
    }
    generateUser(resp);
    response << "Set-Cookie: user=" << resp.getSessionId() << "; HttpOnly; Path=/" << "\r\n";
    response << "Connection: close\r\n";
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

size_t getsizeFile(const string &path)
{
    struct stat st;
    if (stat(path.c_str(), &st) == 0)
    {
        return st.st_size;
    }
    return 0;
}
void chunkedResponse(Response &resp, int clientSocket)
{
    if (resp.getResponseStatus() == First)
    {
        stringstream response;
        response << "HTTP/1.1 " << resp.getStatus() << " " << resp.getStatusValue(resp.getStatus()) << "\r\n";
        response << "Content-type: " << resp.getType() << "\r\n";
        response << "Content Length : " << getsizeFile(resp.getFileName()) << "\r\n";
        generateUser(resp);
        response << "Set-Cookie: user=" << resp.getSessionId() << "; HttpOnly; Path=/" << "\r\n";
        response << "Connection: close\r\n\r\n";
        int bytesend;
        bytesend = send(clientSocket, response.str().c_str(), response.str().size(), MSG_NOSIGNAL);
        if (bytesend <= -1)
            resp.setResponseStatus(Finish);
        if (bytesend != (int)response.str().size() && bytesend != -1)
            resp.setRestSend(response.str().substr(bytesend));
        resp.setResponseStatus(chunked);
    }
    else if (resp.getResponseStatus() == chunked)
    {
        stringstream response;
        response << resp.getBody();
        string responseStr = response.str();
        int bytesend;
        bytesend = send(clientSocket, responseStr.c_str(), responseStr.size(), MSG_NOSIGNAL);
        if (bytesend != (int)response.str().size() && bytesend != -1)
            resp.setRestSend(response.str().substr(bytesend));
        if (bytesend <= 0)
            resp.setResponseStatus(Finish);
    }
    else if (resp.getResponseStatus() == Last)
    {
        stringstream response;
        response << "";
        int bytesend;
        bytesend = send(clientSocket, response.str().c_str(), response.str().size(), MSG_NOSIGNAL);
        if (bytesend <= 0)
            resp.setResponseStatus(Finish);
        resp.setResponseStatus(Finish);
    }
}
void SendResponse(Response &resp, int clientSocket)
{
    if (resp.getResponseStatus() == Nonchunked)
        NonchunkedResponse(resp, clientSocket);
    else
        chunkedResponse(resp, clientSocket);
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
void CleanClient(std::map<int, Client *> &clients, int clientSocket)
{
    if (clients[clientSocket]->getResp()->getResponseStatus() == Finish)
        clients[clientSocket]->setStatus(Finished);
    if (clients[clientSocket]->getStatus() == Finished)
    {
        if (epoll_ctl(clients[clientSocket]->getEpollFd(), EPOLL_CTL_DEL, clientSocket, NULL) == -1)
        {
            perror("epoll_ctl: add");
            return;
        }
        close(clientSocket);
        delete clients[clientSocket];
        clients.erase(clientSocket);
    }
}
void handleClientRequest(std::map<int, Client *> &clients, int clientSocket, std::auto_ptr<std::vector<ConfigFile> > &servers, int epollFd, std::map<int, CgiProcess *> &cgis)
{
    if (clients.find(clientSocket) == clients.end())
        return;
    clients[clientSocket]->updateActivity();
    Client *client = clients[clientSocket];
    try
    {
        clients[clientSocket]->getResp()->initStatusCode();
        if (clients[clientSocket]->getEvent().events & EPOLLIN)
            clients[clientSocket]->ParseHttpRequest(*clients[clientSocket], clientSocket, servers);
        if ((clients[clientSocket]->getStatus() == Processing || clients[clientSocket]->getStatus() == Sending) && clients[clientSocket]->getResp()->getResponseStatus() != Finish)
        {
            clients[clientSocket]->buildResponse(clientSocket, epollFd, cgis);
            clients[clientSocket]->getEvent().events = EPOLLIN | EPOLLOUT;
            if (epoll_ctl(epollFd, EPOLL_CTL_MOD, clientSocket, &clients[clientSocket]->getEvent()) == -1)
            {
                perror("epoll_ctl: add");
                return;
            }
        }
    }
    catch (const exception &e)
    {
        client->getResp()->setRequest(*client->getRequest());
        client->setStatus(Sending);
        client->getEvent().events = EPOLLOUT;
        if (epoll_ctl(epollFd, EPOLL_CTL_MOD, clientSocket, &client->getEvent()) == -1)
        {
            perror("epoll_ctl: add");
            return;
        }
        clients[clientSocket]->getResp()->setRequest(*clients[clientSocket]->getRequest());
        clients[clientSocket]->setStatus(Sending);
        if (atoi(e.what()) == -1)
            clients[clientSocket]->getResp()->setResponseStatus(Finish);
        else
            setCodeStatus(*clients[clientSocket]->getResp(), atoi(e.what()));
    }

    if ((clients[clientSocket]->getEvent().events & EPOLLOUT) && (clients[clientSocket]->getStatus() == Sending))
    {
        SendResponse(*clients[clientSocket]->getResp(), clientSocket);
        clients[clientSocket]->setNewSessionId(clients[clientSocket]->getResp()->getSessionId());
    }
    CleanClient(clients, clientSocket);
}

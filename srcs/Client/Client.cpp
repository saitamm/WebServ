#include "../../Includes/Client.hpp"

vector<string> Client::_session;
Client::Client()
{
    srand(time(0));
    _status = Heading;
    _req = new Request();
    _resp = new Response();
    _timeout = time(NULL);
    memset(&_event, 0, sizeof(_event));
    lastActivity = time(NULL);
}

Client::~Client()
{
    delete _req;
    delete _resp;
}

Client::Client(int fd)
{
    _fd = fd;
}

void Client::setBuff(string &buff, size_t &readbyte) { _buffer.append(buff, readbyte); }
void Client::setKeepAlive(bool ka) { keepAlive = ka; }
void Client::setEpollFd(int fd) { epollFd = fd; }
void Client::setStatus(const ClientStatus &status) { _status = status; }
void Client::setNewSessionId(string &id)
{
    if (find(_session.begin(), _session.end(), id) == _session.end())
        _session.push_back(id);
}
void Client::setResp(Response &resp)
{
    if (_resp)
        delete _resp;
    _resp = &resp;
}

int Client::getFd(void) const { return (_fd); }
time_t Client::getLastActivity() const { return lastActivity; }
Response *Client::getResp(void) { return (_resp); }
epoll_event &Client::getEvent(void) { return (_event); }
int Client::getEpollFd(void) { return (epollFd); }
bool Client::getKeepAlive() const { return keepAlive; }
ssize_t Client::getTimeout(void) const { return _timeout; }
ClientStatus Client::getStatus(void) const { return _status; }

int checkSize(unsigned long long size, size_t max_size)
{
    if (size > max_size)
        return (1);
    return (0);
}
void Client::updateActivity() { lastActivity = time(NULL); }
void Client::buildResponse(int clientFd, int epollFd, map<int, CgiProcess *> &cgis)
{

    this->_resp->setRequest(*this->_req);
    if (!allowMethod(this->_req->getLocation(), this->_req->getMethod()))
    {
        setCodeStatus(*this->getResp(), 405);
        _status = Sending;
        return;
    }
    if (checkSize(this->_req->getContentLength(), this->_req->getConfigFile().getMax_size()))
    {
        setCodeStatus(*this->getResp(), 413);
        _status = Sending;
        return;
    }
    if (this->_req->getMethod() == "DELETE")
    {
        handleDelete(*this->_resp);
        _status = Sending;
        return;
    }
    if (this->_req->getMethod() == "GET")
    {
        if (handleGet(*this->_resp, clientFd, epollFd, cgis) == 0)
        {
            _status = Sending;
        }
        else
        {
            _status = WaitingCGI;
        }
        return;
    }
    if (this->_req->getMethod() == "POST")
    {
        int retur = handlePost(*this->_resp, clientFd, epollFd, cgis);
        if (retur == 0)
        {
            _status = Sending;
        }
        else if (retur == 2)
        {
            _status = WaitingCGI;
        }
        return;
    }
}
void matchServer(Client &client, auto_ptr<vector<ConfigFile> > &serv)
{
    client.getRequest()->setConfigFile(serv->at(0));
    for (int i = 0; i < (int)serv->size(); i++)
    {

        if (serv->at(i).getName() == client.getRequest()->getHost() && serv->at(i).getPort() == client.getRequest()->getPort())
        {
            client.getRequest()->setConfigFile(serv->at(i));
            break;
        }
    }
    client.getRequest()->setLocation(matchLocation(client.getRequest()->getUri(), client.getRequest()->getConfigFile().getLocations()));
}
void Client::ParseHttpRequest(Client &client, int clientSocket, auto_ptr<vector<ConfigFile> > &serv)
{
    char buf[1024];
    ssize_t bytesRead;
    if (_status == Heading)
    {
        if ((bytesRead = recv(clientSocket, buf, sizeof(buf), 0)) >= 0)
            _buffer.append(buf, bytesRead);
        else
        {
            cout << "recv error" << endl;
            _status = Finished;
        }
        if (_buffer.find("\r\n\r\n") != string::npos)
        {
            client.getRequest()->ParseHeader(_buffer);
            matchServer(client, serv);
            RedirectionRequest(*client.getRequest());
            _status = Body;
        }
    }
    if (_status == Body || _status == Reading)
    {
        this->_resp->setRequest(*this->_req);
        if (client.getRequest()->getMethod() == "GET" || client.getRequest()->getMethod() == "DELETE")
        {
            _status = Processing;
            return;
        }
        if (_status == Body)
        {
            CreatUploadFile(*_resp);
            _status = Reading;
        }
        if (ReadBody(*_resp, clientSocket))
        {
            _status = Processing;
            return;
        }
        return;
    }
}
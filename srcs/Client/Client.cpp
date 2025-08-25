#include "../../Includes/Client.hpp"

vector<string> Client::_session;
Client::Client()
{
    srand(time(0));
    _status = Heading;
    _req = new Request();
    _resp = new Response();
    memset(&_event, 0, sizeof(_event));
}

Client::~Client()
{
}

Client::Client(int fd)
{
    _fd = fd;
}

void Client::setResp(Response &resp)
{
    _resp = &resp;
}

int Client::getFd(void) const
{
    return (_fd);
}

Response *Client::getResp(void)
{
    return (_resp);
}

void Client::setBuff(string &buff, size_t &readbyte)
{
    _buffer.append(buff, readbyte);
}

int checkSize(unsigned long long size, size_t max_size)
{
    if (size > max_size)
        return (1);
    return (0);
}
void Client::buildResponse(void)
{
    if (!allowMethod(*this->_req->getLocation(), this->_req->getMethod()))
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
        handleGet(*this->_resp);
        _status = Sending;
        return;
    }
    if (this->_req->getMethod() == "POST")
    {
        _status = Sending;
        return;
    }
}

void Client::ParseHttpRequest(Client &client, int clientSocket, vector<ConfigFile> &serv)
{
    char buf[1024];
    ssize_t bytesRead;
    if (_status == Heading)
    {
        if ((bytesRead = recv(clientSocket, buf, sizeof(buf), 0)) > 0)
            _buffer.append(buf, bytesRead);
        if (_buffer.find("\r\n\r\n") != string::npos)
        {
            client.getRequest()->ParseHeader(_buffer);
            // matching server and location
            this->_req->setConfigFile(serv[0]);
            for (int i = 0; i < (int)serv.size(); i++)
            {
                if (serv[i].getHost() == client.getRequest()->getHost() && serv[i].getPort() == client.getRequest()->getPort())
                {
                    client.getRequest()->setConfigFile(serv[i]);
                    break;
                }
            }
            client.getRequest()->setLocation(matchLocation(client.getRequest()->getUri(), client.getRequest()->getConfigFile().getLocations()));
            if (!client.getRequest()->getLocation())
            {
                cout << "No matching location found for URI: " << client.getRequest()->getUri() << endl;
                throw BadRequestException();
            }
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
            srand(time(0));
            stringstream ll;
            string type = _resp->getRequest()->getHeadvalue("Content-Type").substr(_resp->getRequest()->getHeadvalue("Content-Type").find('/') + 1);
            ll << rand();
            string f = ll.str() + "." + type;
            if (type.empty())
                throw BadRequestException();
            string Up = _resp->getRequest()->getConfigFile().getRoot() + "/" + _resp->getRequest()->getLocation()->getUp_store() + "/" + f;
            _resp->getFile().open(Up.c_str(), ios::out | ios::trunc | ios::binary);
            _resp->setFileName(Up);
            if (!_resp->getFile().is_open())
            {
                setCodeStatus(*this->_resp, 500);
                // cout << "this is my file name: " << _resp->getFileName() << endl;   
                _status = Processing;
                return;
            }
            _status = Reading;
        }
        if (SupportUpload(*_resp))
        {
            setCodeStatus(*_resp, 403);
            _status = Processing;
        }
        if (_resp->getRequest()->getHeadvalue("Transfer-Encoding").empty())
        {
            NonChunkedBody(*_resp, clientSocket);
            if (_resp->getTotalReceived() == _resp->getRequest()->getContentLength())
            {
                if (_resp->getFile().is_open())
                    _resp->getFile().close();
                setCodeStatus(*_resp, 200);
                _status = Processing;
            }
        }
        else
        {
            if (ChunkedBody(*_resp, clientSocket))
            {
                if (_resp->getFile().is_open())
                    _resp->getFile().close();
                setCodeStatus(*_resp, 200);
                _status = Processing;
            }
        }
    }
}

void Client::setNewSessionId(string &id)
{
    if (find(_session.begin(), _session.end(), id) == _session.end())
        _session.push_back(id);
}



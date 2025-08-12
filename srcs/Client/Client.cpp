#include "../../Includes/Client.hpp"

// map<int, string> Client::_StatusCode =
// {
//     {200, "OK"},
//     {204, "No Content"},
//     {403, "Forbidden"},
//     {404, "Not Found"},
//     {405, "Method Not Allowed"},
//     {409, "Conflict"},
//     {500, "Internal Server Error"}
// };

Client::Client()
{
    srand(time(0));
    _status = Heading;
    // stringstream ss;
    // ss << "/tmp/body_" << rand() << ".txt";
    // string filename = ss.str();
    // _body.open(filename.c_str(), std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
    // if (!_body.is_open())
    // {
    //     throw std::runtime_error("Failed to open file: " + filename);
    // }
    _req = new Request();
    _resp = new Response();
}

Client::~Client()
{
}

Client::Client(int fd)
{
    _fd = fd;
    // srand(time(0));
    // stringstream ll;
    // ll << rand();
    // string filename = "Body/body_" + ll.str() + ".txt";
    // _body.open(filename.c_str(), std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
    // if (!_body.is_open())
    // {
    //     throw std::runtime_error("Failed to open file: " + filename);
    // }
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
void Client::buildResponse(int clientSocket)
{
    this->_resp->setRequest(*this->_req);
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
    if (this->_req->getMethod() == "POST" && (_status == Body || _status == Processing))
    {
        if (_status == Body)
        {
            srand(time(0));
            stringstream ll;
            string type = _resp->getRequest()->getHeadvalue("Content-Type").substr(_resp->getRequest()->getHeadvalue("Content-Type").find('/') + 1);
            ll << rand();
            string f = ll.str() + "." + type;
            string Up = _resp->getRequest()->getConfigFile().getRoot() + "/" + _resp->getRequest()->getLocation()->getUp_store() + "/" + f;
            _resp->getFile().open(Up.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
            if (!_resp->getFile().is_open())
            {
                cerr << "soummaya\n";
                setCodeStatus(*this->_resp, 500);
                _status = Sending;
                return;
            }
            _status = Processing;
        }
        if (handlePost(*this->_resp, clientSocket))
        {
            _status = Sending;
            cout << "Post request handled successfully." << endl;
        }
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
        if (_buffer.find("\r\n\r\n") != std::string::npos)
        {
            client.getRequest()->ParseHeader(_buffer);
            // matching server and location
            
            this->_req->setConfigFile(serv[0]);
            for (int i = 0; i < (int)serv.size(); i++)
            {
                if (serv[i].getHost() == client.getRequest()->getHost() && serv[i].getPort() == client.getRequest()->getPort())
                {
                    cout << "Server matched: " << serv[i].getName() << endl;
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
    if (_status == Body)
    {
        if (client.getRequest()->getMethod() == "GET" || client.getRequest()->getMethod() == "DELETE")
        {
            _status = Processing;
            return;
        }
    }
}
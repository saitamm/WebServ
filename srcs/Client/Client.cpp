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
map<int, string> Client::_StatusCode;

void Client::initStatusCodes()
{
    _StatusCode[200] = "OK";
    _StatusCode[204] = "No Content";
    _StatusCode[403] = "Forbidden";
    _StatusCode[404] = "Not Found";
    _StatusCode[405] = "Method Not Allowed";
    _StatusCode[409] = "Conflict";
    _StatusCode[500] = "Internal Server Error";
    // Add other status codes here
}
Client::Client()
{
    srand(time(0));
    stringstream ss;
    ss << "/tmp/body_" << rand() << ".txt";
    string filename = ss.str();
    _body.open(filename.c_str(), std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
    if (!_body.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    _req = new Request();
    _resp = new Response();
}

Client::~Client()
{
    if (_body.is_open())
        _body.close();
}

Client::Client(int fd)
{
    _fd = fd;
    srand(time(0));
    stringstream ll;
    ll << rand();
    string filename = "Body/body_" + ll.str() + ".txt";
    _body.open(filename.c_str(), std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
    if (!_body.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filename);
    }
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

void Client::buildResponse(void)
{
    this->_resp->setRequest(*this->_req);
    if (!allowMethod(*this->_req->getLocation(), this->_req->getMethod()))
    {
        setCodeStatus(*this->getResp(), 405);
        this->_resp->setSend();
        return;
    }
    if (this->_req->getMethod() == "DELETE")
    {
        handleDelete(*this->_resp);
        this->_resp->setSend();
        return;
    }
    if (this->_req->getMethod() == "GET")
    {
        handleGet(*this->_resp);
        this->_resp->setSend();
        return;
    }
    if (this->_req->getMethod() == "POST" && this->_req->getfinishedBody())
    {
        handlePost(*this->_resp);
        this->_resp->setSend();
        return;
    }
}

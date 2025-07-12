#include "../hpp/Client.hpp"

Client::Client()
{
    srand(time(0));
    stringstream ss;
    ss << "Body/body_" << rand() << ".txt";
    string filename = ss.str();
    _body.open(filename.c_str(), std::ios::out | std::ios::trunc);
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
    _body.open(filename.c_str(), std::ios::out | std::ios::trunc);
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
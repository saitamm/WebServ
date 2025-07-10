#include"../hpp/Client.hpp"


Client::Client(){}


Client::Client(int fd)
{
    _fd = fd;
}

void Client::setResp(Response &resp)
{
    _resp = resp;
}


int Client::getFd(void) const
{
    return (_fd);
}


Response &Client::getResp(void)
{
    return (_resp);
}
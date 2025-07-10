#ifndef CLIENT_HPP
#define CLIENT_HPP
#include "Response.hpp"

class Client
{
public:
    Client();
    Client(int fd);
    Response &getResp(void);
    void setResp(Response &rep);
    int getFd(void) const;

private:
    Response _resp;
    int _fd;
    // bool _finished;
};
void ServClient(map<int, Client> &clients, int clientSocket, vector<ConfigFile> *servers);
#endif
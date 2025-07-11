#ifndef CLIENT_HPP
#define CLIENT_HPP
#include "Response.hpp"

class Client
{
public:
    Client();
    Client(int fd);
    Response *getResp(void);
    void setResp(Response &rep);
    void setBuff(string &buff, size_t &readbyte);
    int getFd(void) const;
    string &getbuff(void){return(_buffer);}

private:
    Response *_resp;
    int _fd;
    string _buffer;
    fstream _file;
    // bool _finished;
};
void ServClient(map<int, Client> &buffers, int clientSocket, vector<ConfigFile> *servers);
#endif
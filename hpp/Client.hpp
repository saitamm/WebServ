#ifndef CLIENT_HPP
#define CLIENT_HPP
#include "Response.hpp"

class Response;
class Client
{
public:
    Client();
    Client(int fd);
    ~Client();
    Response *getResp(void);
    void setResp(Response &rep);
    void setBuff(string &buff, size_t &readbyte);
    int getFd(void) const;
    string &getbuff(void) { return (_buffer); }
    fstream &getbody(void) { return (_body); }

private:
    Client(const Client &copy) ;
    Client &operator=(Client &hs) ;
    Response *_resp;
    int _fd;
    string _buffer;
    fstream _body;
    // bool _finished;
};
void ServClient(map<int, Client*> &buffers, int clientSocket, vector<ConfigFile> *servers);
#endif
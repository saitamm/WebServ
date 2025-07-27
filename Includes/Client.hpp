#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Response.hpp"

enum ClientStatus
{
    CONNECTING,
    Heading,
    Body,
    Processing,
    Sending,
    Finished,
};
class ConfigFile;
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
    Request *getRequest(void){return(_req);};
    void buildResponse(void);
    void setStatus(const ClientStatus &status) { _status = status; }
    ClientStatus getStatus(void) const { return _status; }
    void ParseHttpRequest(Client &client,int clientSocket ,ConfigFile &serv);

private:
    Client(const Client &copy) ;
    Request *_req;
    Response *_resp;
    int _fd;
    string _buffer;
    fstream _body;
    ClientStatus _status;
    
};
int allowMethod(Location loc, string method);
void printRequest(Request req);
void handleClientRequest(map<int, Client *> &clients, int clientSocket, vector<ConfigFile> *servers);

#endif
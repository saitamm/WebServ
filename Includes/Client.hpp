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
    Request *getRequest(void){return(_req);};
    void buildResponse(int clientSocket);
    void setStatus(const ClientStatus &status) { _status = status; }
    ClientStatus getStatus(void) const { return _status; }
    void ParseHttpRequest(Client &client,int clientSocket ,vector<ConfigFile> &serv);
    void setNewSessionId(string &id);
    string &getSession(void);

private:
    Client(const Client &copy) ;
    Request *_req;
    Response *_resp;
    int _fd;
    string _buffer;
    ClientStatus _status;
    static vector<string> _session;
};
int allowMethod(Location loc, string method);
// void printRequest(Request req);
void handleClientRequest(map<int, Client *> &clients, int clientSocket, vector<ConfigFile> *servers);

#endif
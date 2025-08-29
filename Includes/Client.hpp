#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Response.hpp"
#include <stdexcept>

enum ClientStatus
{
    CONNECTING,
    Heading,
    Body,
    Processing,
    WaitingCGI,
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
    void buildResponse(int clientSocket, int epollFd, map<int, CgiProcess*> &cgis);
    void setStatus(const ClientStatus &status) { _status = status; }
    ClientStatus getStatus(void) const { return _status; }
    void ParseHttpRequest(Client &client,int clientSocket ,ConfigFile &serv);
    private:
    Client(const Client &copy) ;
    Request *_req;
    Response *_resp;
    int _fd;
    string _buffer;
    ClientStatus _status;
    
};
int allowMethod(Location loc, string method);
void printRequest(Request req);
void handleClientRequest(map<int, Client *> &clients, int clientSocket, vector<ConfigFile> *servers, int epollFd, map<int, CgiProcess*>&cgis);
void SendResponse(Response &resp, int clientSocket);
void setNonBlocking(int fd);
void checkCgiGet(Response &resp, string &real_path, int clientFd, int epollFd, map<int, CgiProcess*> &cgis);
void checkCgiPost(Response& resp, int clientFd, int epollFd, map<int, CgiProcess*> &cgis);
void CgiEvent(int fd, int epollFd, map<int, Client *> &clients, map<int, CgiProcess *> &cgis);
string getExt(Response &resp);
bool isCgiExtension(const string &ext, Response &resp);

#endif
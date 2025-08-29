#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Response.hpp"

enum ClientStatus
{
    CONNECTING,
    Heading,
    Body,
    Reading,    
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
    void buildResponse(void);
    void setStatus(const ClientStatus &status) { _status = status; }
    ClientStatus getStatus(void) const { return _status; }
    void ParseHttpRequest(Client &client,int clientSocket ,vector<ConfigFile> &serv);
    void setNewSessionId(string &id);
    string &getSession(void);
    epoll_event &getEvent(void) { return (_event); }
    static int getEpollFd(void) { return (epollFd); }
    static void setEpollFd(int fd) { epollFd = fd; }
    
    void setNonBlocking(int fd);
private:
    Client(const Client &copy) ;
    Request *_req;
    Response *_resp;
    int _fd;
    string _buffer;
    ClientStatus _status;
    static vector<string> _session;

    //Test
    epoll_event _event;
    static int epollFd;
};
int allowMethod(Location loc, string method);
// void printRequest(Request req);
void handleClientRequest(map<int, Client *> &clients, int clientSocket, vector<ConfigFile> *servers);

#endif
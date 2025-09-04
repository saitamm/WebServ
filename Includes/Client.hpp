#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Response.hpp"
#include <stdexcept>
#include <csignal>

enum ClientStatus
{
    CONNECTING,
    Heading,
    Body,
    Reading,
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
    Request *getRequest(void) { return (_req); };
    void ParseHttpRequest(Client &client, int clientSocket, auto_ptr<vector<ConfigFile> > &serv);
    void setNewSessionId(string &id);
    string &getSession(void);
    epoll_event &getEvent(void) { return (_event); }
    static int getEpollFd(void) { return (epollFd); }
    static void setEpollFd(int fd) { epollFd = fd; }

    void setNonBlocking(int fd);
    void buildResponse(int clientSocket, int epollFd, map<int, CgiProcess *> &cgis);
    void setStatus(const ClientStatus &status) { _status = status; }
    ClientStatus getStatus(void) const { return _status; }
    ssize_t getTimeout(void) const { return _timeout; }

    void updateActivity() { lastActivity = time(NULL); }
    time_t getLastActivity() const { return lastActivity; }
    void setKeepAlive(bool ka) { keepAlive = ka; }
    bool getKeepAlive() const { return keepAlive; }
private:
    Client(const Client &copy);
    Request *_req;
    Response *_resp;
    int _fd;
    string _buffer;
    ClientStatus _status;
    static vector<string> _session;
    epoll_event _event;
    static int epollFd;
    ssize_t _timeout;
    time_t lastActivity;
    bool keepAlive;
    // Test
};
int allowMethod(Location loc, string method);
void handleClientRequest(map<int, Client *> &clients, int clientSocket, auto_ptr<vector<ConfigFile> > &servers, int epollFd, map<int, CgiProcess *> &cgis);
void SendResponse(Response &resp, int clientSocket);
void setNonBlocking(int fd);
void checkCgiGet(Response &resp, string &real_path, int clientFd, int epollFd, map<int, CgiProcess *> &cgis, string &ext);
void checkCgiPost(Response &resp, int clientFd, int epollFd, map<int, CgiProcess *> &cgis, string& ext);
void CgiEvent(int fd, int epollFd, map<int, Client *> &clients, map<int, CgiProcess *> &cgis);
string getExt(Response &resp);
bool isCgiExtension(const string &ext, Response &resp);

#endif
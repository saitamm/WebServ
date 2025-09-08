#ifndef RESPONSE_HPP
#define RESPONSE_HPP
#include "Request.hpp"
#include <algorithm>
#include <sys/wait.h>

#include "CgiProcess.hpp"

class CgiProcess;


enum ResponseStatus
{
    chunked,
    Nonchunked,
    First,
    Last,
    Send,
    Finish
};
class Response
{
public:
    Response();
    ~Response();
    // getters
    int getStatus(void) const;
    string &getContenttype(void);
    Request *getRequest(void);
    string &getStatusValue(int key);
    string &getBody(void);
    string &getType();
    fstream &getFile(void);
    unsigned int getReceived(void);
    unsigned int getBufferSize(void);
    string &getFileName(void);
    string &getSessionId(void);
    fstream &getChunkFile(void);
    string &getRestPost(void);
    ResponseStatus getResponseStatus(void) const;
    string &getRestSend(void);
    // setters
    void setStatus(int stat);
    void setRequest(Request &req);
    void setContentType(string content);
    void setBodyResp(const string &body);
    void setType(const string &type);
    size_t getTotalReceived(void);
    void setTotalReceived(size_t received);
    void setReceived(unsigned int received);
    void restartChunk(void);
    void setBufferSize(unsigned int size);
    void setSessionId(string &user);
    void setFileName(const string &filename);
    void setResponseStatus(ResponseStatus status);
    void setChunkFile(string &name);
    void setRestSend(const string &rest);
    void setRestPost(const string &rest);
    // Test
    void initStatusCode(void);

private:
    string _ContentType;
    string _body;
    Request *_req;
    string _type;
    static map<int, string> _StatusCode;
    int _Code;
    fstream _file;
    size_t totalReceived;
    string _sessionId;
    string _filename;
    unsigned int _received;
    unsigned int bufferSize;
    string _restPost;
    ResponseStatus _status;
    fstream _chunkFile;
    string _restSend;
};
void handleDelete(Response &resp);
int handleGet(Response &resp, int clientFd, int epollFd, map<int, CgiProcess *> &cgis);
void setCodeStatus(Response &resp, int error);
void getContentType(string &real_path, Response &resp);
void NonChunkedBody(Response &resp, int clientSocket);
int ChunkedBody(Response &resp, int clientSocket);
int SupportUpload(Response &resp);
size_t getFileSize(const string &path);
int handlePost(Response &resp, int clientSocket, int epollFd, map<int, CgiProcess *> &cgis);
string checkCgiPath(Response &resp);
void CreatUploadFile(Response &resp);
int ReadBody(Response &resp, int clientSocket);

#endif
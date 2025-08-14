#ifndef RESPONSE_HPP
#define RESPONSE_HPP
#include "Request.hpp"
#include <algorithm>
#include<sys/wait.h>
class BadDirectoryException : public std::exception
{
public:
    const char *what() const throw()
    {
        return ("soumayaaaaaaaaaaaa\n");
    }
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
    fstream &getFile(void );
    unsigned int getReceived(void);
    unsigned int getBufferSize(void);
    string &getSessionId(void);
    // setters
    void setStatus(int stat);
    void setRequest(Request &req);
    void setContentType(string content);
    void setBodyResp(string &body);
    void setType(const string &type);
    void initStatusCode(void);
    size_t getTotalReceived(void);
    void setTotalReceived(size_t received);
    void setReceived(unsigned int received);
    void restartChunk(void);
    void setBufferSize(unsigned int size);
    void setSessionId(string  &user);
    void setFileName(const string &filename) { _filename = filename; }
    string &getFileName(void) { return _filename; }

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

    //chunked body
    unsigned int _received;
    unsigned int bufferSize;
};
void handleDelete(Response &resp);
void handleGet(Response &resp);
void setCodeStatus(Response &resp, int error);
void getContentType(string &real_path, Response &resp);
int handlePost(Response &resp, int clientSocket);
#endif
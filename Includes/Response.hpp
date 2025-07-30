#ifndef RESPONSE_HPP
#define RESPONSE_HPP
#include "Request.hpp"
#include <algorithm>
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
    bool getSend() ;
    fstream &getFile(void );
    // setters
    void setStatus(int stat);
    void setRequest(Request &req);
    void setContentType(string content);
    void setBodyResp(string &body);
    void setType(const string &type);
    void setSend(void);
    void initStatusCode(void);
    size_t getTotalReceived(void);
    void setTotalReceived(size_t received);

private:
    string _ContentType;
    string _body;
    Request *_req;
    string _type;
    bool _sendit;
    static map<int, string> _StatusCode;
    int _Code;
    fstream _file;
    size_t totalReceived;
};
void handleDelete(Response &resp);
void handleGet(Response &resp);
void setCodeStatus(Response &resp, int error);
void getContentType(string &real_path, Response &resp);
int handlePost(Response &resp, int clientSocket);
#endif
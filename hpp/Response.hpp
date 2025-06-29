#ifndef RESPONSE_HPP
#define RESPONSE_HPP
#include "Request.hpp"
#include <sys/stat.h>

class Response
{
public:
    Response();
    ~Response();
    //getters
    int getStatus(void) const;
    size_t &getContentLength(void);
    string &getContenttype(void);
    Request &getRequest(void);
    string &getValue(string key);
    string &getBody(void);
    //setters
    void setStatus(int stat);
    void setContentLength(size_t size);
    void setContentType(string type);
    void setRequest(Request &req);
    void setBody(string &body);
    void setError(string key, string value);
    // void MakeResponce(Request &req);

private:
    int _status;
    size_t _ContentLength;
    string _ContentType;
    map<string, string> _Error;
    string _body;
    Request _req;
};

void MakeResponce(Request &req, Response &resp);
void handleDelete(Response &resp);
void setErrorBodyStatus(Response &resp, int error);
#endif
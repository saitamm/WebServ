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
    string getStatus(void) const;
    size_t &getContentLength(void);
    string &getContenttype(void);
    Request &getRequest(void);
    string &getValue(string key);
    string &getBody(void);
    //setters
    void setStatus(string stat);
    void setContentLength(size_t size);
    void setContentType(string type);
    void setRequest(Request &req);
    void setBody(string &body);
    void setError(string key, string value);
    // void MakeResponce(Request &req);

private:
    string _status;
    size_t _ContentLength;
    string _ContentType;
    map<string, string> _Error;
    string _body;
    Request _req;
};

void MakeResponce(Request &req, Response &resp);
void handleDelete(Response &resp);
#endif
#ifndef RESPONSE_HPP
#define RESPONSE_HPP
#include "Request.hpp"
#include <sys/stat.h>
#include <unistd.h> 
#include <dirent.h>

class Response
{
public:
    Response();
    ~Response();
    //getters
    int getStatus(void) const;
    string &getContenttype(void);
    Request &getRequest(void);
    string &getValue(int key);
    string &getBody(void);
    string & getType();
    //setters
    void setStatus(int stat);
    void setRequest(Request &req);
    void setContentType(string content) ;
    void setBody(string &body);
    void setError(int key, string value);
    void setType(const string &type);
    // void MakeResponce(Request &req);

private:
    int _status;
    string _ContentType;
    map<int, string> _Error;
    string _body;
    Request _req;
    string _type;
};

void MakeResponce(Request &req, Response &resp);
void handleDelete(Response &resp);
void handleGet(Response &resp);
void setErrorBodyStatus(Response &resp, int error);
#endif
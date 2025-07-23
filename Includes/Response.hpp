#ifndef RESPONSE_HPP
#define RESPONSE_HPP
#include "Request.hpp"

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
    bool getSend() { return (_sendit); };
    // setters
    void setStatus(int stat);
    void setRequest(Request &req);
    void setContentType(string content);
    void setBody(string &body);
    void setType(const string &type);
    void setSend(void) { _sendit = true; };
    void initStatusCode(void);
    // void buildResponsee(Request &req);

private:
    int _status;
    string _ContentType;
    string _body;
    Request *_req;
    string _type;
    bool _sendit;
    static map<int, string> _StatusCode;
};
void handleDelete(Response &resp);
void handleGet(Response &resp);
void setCodeStatus(Response &resp, int error);
void getContentType(string &real_path, Response &resp);
void handlePost(Response &resp);
#endif
#include "../hpp/Response.hpp"

Response::Response() {}
Response::~Response() {}

int allowMethod(Location loc, string method)
{
    for (std::set<std::string>::const_iterator it = loc.getMethods().begin(); it != loc.getMethods().end(); ++it)
    {
        if (*it == method)
            return (1);
    }
    return (0);
}

void MakeResponce(Request &req, Response &resp)
{
    resp.setRequest(req);
    if (!allowMethod(*req.getLocation(), req.getMethod()))
    {
        setErrorBodyStatus(resp, 405);
        resp.setSend();
        return;
    }
    if (req.getMethod() == "DELETE")
    {
        handleDelete(resp);
        resp.setSend();
        return;
    }
    if (req.getMethod() == "GET")
    {
        handleGet(resp);
        resp.setSend();
        return;
    }
    if (req.getMethod() == "POST" && req.getfinishedBody())
    {
        handlePost(resp);
        resp.setSend();
        return;
    }
}

int Response::getStatus(void) const { return (_status); }
string &Response::getContenttype(void) { return (_ContentType); }
Request *Response::getRequest(void) { return (_req); }
string &Response::getValue(int key) { return (_Error[key]); }
string &Response::getBody(void) { return (_body); }
string &Response::getType() { return _type; }

// setters
void Response::setContentType(string content) { _ContentType = content; }
void Response::setRequest(Request &req) { _req = &req; }
void Response::setStatus(int stat) { _status = stat; }
void Response::setBody(string &body) { _body = body; }
void Response::setError(int key, string value) { _Error[key] = value; }
void Response::setType(const string &type) { _type = type; }

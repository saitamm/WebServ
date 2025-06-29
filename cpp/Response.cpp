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
    resp.setError("200", "OK");
    resp.setError("204", "No Content");
    resp.setError("403", "Forbidden");
    resp.setError("404", "Not Found");
    resp.setError("405", "Method Not Allowed");
    if (!allowMethod(*req.getLocation(), req.getMethod()))
    {
        setErrorBodyStatus(resp, 405);
        return;
    }
    if (req.getMethod() == "DELETE")
    {
        handleDelete(resp);
        return;
    }
}

int Response::getStatus(void) const { return (_status); }
size_t &Response::getContentLength(void) { return (_ContentLength); }
string &Response::getContenttype(void) { return (_ContentType); }
Request &Response::getRequest(void) { return (_req); }
string &Response::getValue(string key) { return (_Error[key]); }
string &Response::getBody(void) { return (_body); }

// setters
void Response::setContentLength(size_t size) { _ContentLength = size; }
void Response::setContentType(string content) { _ContentType = content; }
void Response::setRequest(Request &req) { _req = req; }
void Response::setStatus(int stat) { _status = stat; }
void Response::setBody(string &body) { _body = body; }
void Response::setError(string key, string value) { _Error[key] = value; }

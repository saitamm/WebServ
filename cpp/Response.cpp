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
        resp.setStatus("405");
    if (req.getMethod() == "DELETE")
    {
        handleDelete(resp);
        return;
    } 
}

string Response::getStatus(void) const { return (_status); }
size_t &Response::getContentLength(void) { return (_ContentLength); }
string &Response::getContenttype(void) { return (_ContentType); }
Request &Response::getRequest(void) { return (_req); }

// setters
void Response::setContentLength(size_t size) { _ContentLength = size; }
void Response::setContentType(string content) { _ContentType = content; }
void Response::setRequest(Request &req) { _req = req; }
void Response::setStatus(string stat) { _status = stat; }

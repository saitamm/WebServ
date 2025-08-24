#include "../../Includes/Response.hpp"

map<int, string> Response::_StatusCode;

void Response::initStatusCode(void)
{
    _StatusCode[200] = "OK";
    _StatusCode[204] = "No Content";
    _StatusCode[403] = "Forbidden";
    _StatusCode[404] = "Not Found";
    _StatusCode[405] = "Method Not Allowed";
    _StatusCode[409] = "Conflict";
    _StatusCode[500] = "Internal Server Error";
    _StatusCode[300] = "Multiple Choices";
    _StatusCode[301] = "Moved Permanently";
    _StatusCode[302] = "Found";
    _StatusCode[303] = "See Other";
    _StatusCode[304] = "Not Modified";
    _StatusCode[305] = "Use Proxy";
    _StatusCode[306] = "Switch Proxy";
    _StatusCode[307] = "Temporary Redirect";
    _StatusCode[308] = "Permanent Redirect";
    _StatusCode[413] = "Payload Too Large";
    // _StatusCode[400] = "Bad Request";
}

Response::Response()
{
    totalReceived = 0;
    _received = 0;
    _status = Nonchunked;
}
Response::~Response() {}

void Response::restartChunk(void) { _received = 0; }

string &Response::getContenttype(void) { return (_ContentType); }
Request *Response::getRequest(void) { return (_req); }
string &Response::getStatusValue(int key) { return (_StatusCode[key]); }
string &Response::getBody(void) { return (_body); }
string &Response::getType() { return _type; }
int Response::getStatus(void) const { return (_Code); };
fstream &Response::getFile(void) { return (_file); }
size_t Response::getTotalReceived(void) { return totalReceived; }
unsigned int Response::getReceived(void) { return _received; }
unsigned int Response::getBufferSize(void) { return bufferSize; }
string &Response::getSessionId(void) { return (_sessionId); }

// setters
void Response::setContentType(string content) { _ContentType = content; }
void Response::setRequest(Request &req) { _req = &req; }
void Response::setBodyResp(const string &body) { _body = body; }
void Response::setType(const string &type) { _type = type; }
void Response::setStatus(int stat) { _Code = stat; };
void Response::setTotalReceived(size_t received) { totalReceived += received; }
void Response::setReceived(unsigned int received) { _received += received; }
void Response::setBufferSize(unsigned int size) { bufferSize = size; }
void Response::setSessionId(string &user) { _sessionId = user; }

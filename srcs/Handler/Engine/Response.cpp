#include "../../../Includes/Response.hpp"

Response::Response() {}
Response::~Response() {}


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
void Response::setCode(int key, string value) { _Error[key] = value; }
void Response::setType(const string &type) { _type = type; }

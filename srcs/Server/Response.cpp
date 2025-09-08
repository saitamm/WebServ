#include "../../Includes/Response.hpp"

map<int, string> Response::_StatusCode;

void Response::initStatusCode(void)
{
    _StatusCode[200] = "OK";
    _StatusCode[403] = "Forbidden";
    _StatusCode[404] = "Not Found";
    _StatusCode[405] = "Method Not Allowed";
    _StatusCode[409] = "Conflict";
    _StatusCode[500] = "Internal Server Error";
    _StatusCode[300] = "Multiple Choices";
    _StatusCode[301] = "Moved Permanently";
    _StatusCode[302] = "Found";
    _StatusCode[413] = "Payload Too Large";
    _StatusCode[431] = "Payload Too Large";
    _StatusCode[400] = "Bad Request";
    _StatusCode[408] = "Request Timeout";
    _StatusCode[411] = "Length Required";
    _StatusCode[501] = "Not Implemented";
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
string &Response::getFileName(void) { return _filename; }
string &Response::getType() { return _type; }
int Response::getStatus(void) const { return (_Code); };
fstream &Response::getFile(void) { return (_file); }
size_t Response::getTotalReceived(void) { return totalReceived; }
unsigned int Response::getReceived(void) { return _received; }
unsigned int Response::getBufferSize(void) { return bufferSize; }
string &Response::getSessionId(void) { return (_sessionId); }
string &Response::getRestPost(void) { return _restPost; }
ResponseStatus Response::getResponseStatus(void) const { return _status; }
// setters
void Response::setFileName(const string &filename) { _filename = filename; }
void Response::setResponseStatus(ResponseStatus status) { _status = status; }
void Response::setContentType(string content) { _ContentType = content; }
void Response::setRequest(Request &req) { _req = &req; }
void Response::setBodyResp(const string &body) { _body = body; }
void Response::setType(const string &type) { _type = type; }
void Response::setStatus(int stat) { _Code = stat; };
void Response::setTotalReceived(size_t received) { totalReceived += received; }
void Response::setReceived(unsigned int received) { _received += received; }
void Response::setBufferSize(unsigned int size) { bufferSize = size; }
string &Response::getRestSend(void) { return _restSend; }
fstream &Response::getChunkFile(void) { return _chunkFile; }

void Response::setRestPost(const string &rest) { _restPost = rest; }
void Response::setRestSend(const string &rest) { _restSend = rest; }
void Response::setSessionId(string &user) { _sessionId = user; }
void Response::setChunkFile(string &name)
{
    _chunkFile.open(name.c_str(), ios::in | ios::binary);
    if (!_chunkFile.is_open())
    {
        cerr << "Failed to open chunk file: " << name << endl;
    }
}


//function

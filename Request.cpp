#include "Request.hpp"

Request::Request()
{
}

Request::~Request() {}

// vector<string> split(string buf)
// {

// }

void ParseRequest(string buffer, Request &request)
{
    std::cout << "----this is the Request ------\n"
              << buffer << endl;
    cout << request.getMethod() << endl;
    // int i = 0;
    // string line;
    // while ()
}

// setters

string Request::getHost(void) const { return (_host); }
string Request::getMethod(void) const { return (_method); }
string Request::getPath(void) const { return (_path); }
string Request::getPort(void) const { return (_port); }
int Request::getVServer(void) const { return (_Vserver); }
size_t Request::getContentLength(void) const { return (ContentLength); }

// getters

void Request::setHost(string host) { _host = host; }
void Request::setMethod(string meth) { _method = meth; }
void Request::setPath(string path) { _path = path; }
void Request::setPort(string port) { _port = port; }
void Request::setVServer(int val) { _Vserver = val; }
void Request::setContentLength  (size_t Content) { ContentLength = Content; }

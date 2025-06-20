#include "Request.hpp"

Request::Request()
{
}

Request::~Request() {}

// vector<string> split(string buf)
// {

// }

// void ParseRequest(string buffer, Request &request)
// {
//     std::cout << "----this is the Request ------\n"
//               << buffer << endl;
//     cout << request.getMethod() << endl;
//     request.setMethod(buffer.substr(0, buffer.find(' ' , 0)));
//     cout << " -----"<< request.getMethod() << "---"<< endl;
//     int pos = buffer.find(' ', 0);
//     pos++;
//     request.setPath(buffer.substr(pos, buffer.find(' ', pos+1)-pos));
//     cout << " -----"<< request.getPath()<<"---"<< endl;
//     pos = buffer.find(' ' , buffer.find('\n', pos));
//     pos++;
//     request.setHost(buffer.substr(pos, buffer.find(':', pos+1)-pos));
//     cout << "------" << request.getHost() << "------" << endl;
// }

void ParseRequest(int clientSocket, Request &req)
{
    std::string Header;
    char buf[10];
    ssize_t bytesRead;

    while ((bytesRead = recv(clientSocket, buf, sizeof(buf), 0)) > 0)
    {
        Header.append(buf, bytesRead);
        cout << Header << endl;
        cout << "-------------------------------------------\n";
        if (Header.find("Content-Type:") != std::string::npos)
            break;
    }
    //parse header
    req.setMethod(Header.substr(0, Header.find(' ', 0)));
    Header.erase(Header.find(' ', 0)+1);
    std::cout << "---" << req.getMethod() << "------"<< endl;
    cout << "==="<< Header << endl;


    req.setContentLength(8);
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
void Request::setContentLength(size_t Content) { ContentLength = Content; }

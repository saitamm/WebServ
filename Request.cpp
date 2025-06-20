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

void Request::ParseRequest(int clientSocket)
{
    std::string Header;
    char buf[1];
    ssize_t bytesRead;

    while ((bytesRead = recv(clientSocket, buf, sizeof(buf), 0)) > 0)
    {
        Header.append(buf, bytesRead);
        if (Header.find("\r\n\r\n") != std::string::npos)
            break;
    }
    // parse header
    this->_method = Header.substr(0, Header.find(' ', 0));
    Header.erase(0, Header.find(' ', 0) + 1);
    this->_path = Header.substr(0, Header.find(' ', 0));
    Header.erase(0, Header.find('\n', 0) + 1);
    Header.erase(0, Header.find(' ', 0) + 1);
    this->_host = Header.substr(0, Header.find(' ', 0));
    Header.erase(0, Header.find(':', 0) + 1);
    this->_port = Header.substr(0, Header.find('\n', 0));
    Header.erase(0, Header.find('\n', 0) + 1);
    this->_head[Header.substr(0, Header.find(':', 0))] = Header.substr(Header.find(':', 0) + 2, Header.find('\n', 0) - Header.find(':', 0) - 2);
    Header.erase(0, Header.find('\n', 0) + 1);
    this->_head[Header.substr(0, Header.find(':', 0))] = Header.substr(Header.find(':', 0) + 2, Header.find('\n', 0) - Header.find(':', 0) - 2);
    Header.erase(0, Header.find('\n', 0) + 1);
    if (_method == "GET" || _method == "DELETE")
    {
        cout << "soumaaaaaya\n";
        return;
    }
    Header.erase(0, Header.find(':', 0) + 1);
    stringstream ss(Header.substr(0, Header.find('\n', 0)));
    ss >> this->_ContentLength;
    Header.erase(0, Header.find('\n', 0) + 1);
    this->_head[Header.substr(0, Header.find(':', 0))] = Header.substr(Header.find(':', 0) + 2, Header.find(';', 0) - Header.find(':', 0) - 2);
    Header.erase(0, Header.find('=', 0) + 1);
    // parse Body
    string del;

    while (Header[0] == '-')
        Header.erase(0, 1);
    del = Header.substr(0, Header.find('\n', 0));
    if (this->_ContentLength <= LIMIT)
    {
        this->flag = STR;
        string body;
        while ((bytesRead = recv(clientSocket, buf, sizeof(buf), 0)) > 0)
        {
            body.append(buf, bytesRead);
            if (Header.find(del) != std::string::npos)
                break;
        }
        cout << "Body  =" << body << endl;
    }
    else
    {
        char file = "/tmp/body_upload_abc123.tmp";
    }
}

// setters

string Request::getHost(void) const { return (_host); }
string Request::getMethod(void) const { return (_method); }
string Request::getPath(void) const { return (_path); }
string Request::getPort(void) const { return (_port); }
// int Request::getVServer(void) const { return (_Vserver); }
size_t Request::getContentLength(void) const { return (_ContentLength); }

// getters

void Request::setHost(string host) { _host = host; }
void Request::setMethod(string meth) { _method = meth; }
void Request::setPath(string path) { _path = path; }
void Request::setPort(string port) { _port = port; }
// void Request::setVServer(int val) { _Vserver = val; }
void Request::setContentLength(size_t Content) { _ContentLength = Content; }

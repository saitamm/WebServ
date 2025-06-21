#include "Request.hpp"

Request::Request()
{
}

Request::~Request() {}

void trim(string &str, string tr)
{
    std::string::size_type pos = str.find_first_not_of(tr);
    str.erase(0, pos);
    pos = str.find_last_not_of(tr);
    str.erase(pos + 1);
}
void split(string str, char c, vector<string> &resul)
{
    if (str.find(c) == string::npos)
    {
        resul.push_back(str);
        return;
    }
    resul.push_back(str.substr(0, str.find(c)));
    str.erase(0, str.find(c) + 1);
    resul.push_back(str);
}
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
    cout << Header << endl;
    // parse header
    stringstream line(Header);
    line >> this->_method;
    if (_method != "GET" && _method != "DELETE" && _method != "POST")
        throw MethodNotAllowedException();
    string path;
    vector<string> res;
    line >> path;
    split(path, '?', this->_url);
    string Httpv;
    line >> Httpv;
    trim(Httpv, "\n\t\r ");
    if (Httpv != "HTTP/1.1")
        throw ErrorHttpVersionException();
    string tmp;
    line >> tmp;
    line >> this->_host;
    if (this->_host.empty() || tmp.find(':') == string::npos)
        throw HostIssueException();
    if (this->_host.find(':') != string::npos)
        this->_host = this->_host.substr(0, this->_host.find(':'));
    this->_port = 8080;
    while (line >> tmp && tmp.find("boundary") == string::npos)
    {
        if (tmp != "Content-Length:")
        {

            trim(tmp, ":");
            string value;
            line >> value;
            trim(value, "\n\t\r ");
            this->_head[tmp] = value;

        }
        else
        {
            line >> this->_ContentLength;
        }
    }
    if (_method == "GET" || _method == "DELETE")
    {
        cout << "soumaaaaaya\n";
        return;
    }
    // while ((bytesRead = recv(clientSocket, buf, sizeof(buf), 0)) > 0)
    // {
    //     Header.append(buf, bytesRead);
    //     if (Header.find("\r\n\r\n") != std::string::npos)
    //         break;
    // }
}

// setters

string Request::getHost(void) const { return (_host); }
string Request::getMethod(void) const { return (_method); }
// string Request::getPath(void) const { return (_path); }
int Request::getPort(void) const { return (_port); }
// int Request::getVServer(void) const { return (_Vserver); }
size_t Request::getContentLength(void) const { return (_ContentLength); }

// getters

void Request::setHost(string host) { _host = host; }
void Request::setMethod(string meth) { _method = meth; }
// void Request::setPath(string path) { _path = path; }
// void Request::setPort(string port) { _port = port; }
// void Request::setVServer(int val) { _Vserver = val; }
void Request::setContentLength(size_t Content) { _ContentLength = Content; }

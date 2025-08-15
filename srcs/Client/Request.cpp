#include "../../Includes/Request.hpp"

Request::Request()
{
    _redir = false;
}
Request::~Request() {}

// getters && setters

string &Request::getHost(void) { return (_host); }
string &Request::getMethod(void) { return (_method); }
string &Request::getUri(void) { return (_url[0]); }
string &Request::getQuery(void) { return (_url[1]); }
string &Request::getCtype(void) { return (_head["Content-Type"]); }
Location *Request::getLocation(void) { return (_locat); }
string &Request::getHeadvalue(string key) { return (_head[key]); }
unsigned long long &Request::getContentLength(void) { return (_ContentLength); }
ConfigFile &Request::getConfigFile(void) { return (_serv); }
bool Request::getRedirectionStatus(void) const { return _redir; }
string &Request::getrestHeader(void) { return (restHeader); }
int Request::getPort(void)
{
    stringstream ss(_port);
    int port;
    ss >> port;
    return (port);
}
string &Request::getCookie(void) { return _cookie; }

void Request::setMethod(const string &method) { _method = method; }
void Request::setHost(const string &host) { _host = host; }
void Request::setUrl(vector<string> &url) { _url = url; }
void Request::setHeadvalue(const string &key, const string &value) { _head[key] = value; }
void Request::setLocation(Location *locat) { _locat = locat; }
void Request::setHeader(string &key, string &value) { _head[key] = value; }
void Request::setRestHeader(const string &rest) { restHeader = rest; }
void Request::setConfigFile(ConfigFile &serv) { _serv = serv; }
void Request::setRedirectionStatus(void) { _redir = true; }

// Parse Request
void Request::ParseHeader(string &Header)
{

    string tmp1;
    stringstream line(Header);
    line >> this->_method;
    if ((_method != "GET" && _method != "DELETE" && _method != "POST") || _method.empty())
        throw BadRequestException();
    string path;
    vector<string> res;
    line >> path;
    split(path, '?', this->_url);
    string Httpv;
    line >> Httpv;
    trim(Httpv, "\n\t\r ");
    if (Httpv != "HTTP/1.1")
        throw BadRequestException();
    string tmp;
    _ContentLength = 0;
    getline(line, tmp);
    string input;
    while (getline(line, input) && input != "\r")
    {
        tmp = input.substr(0, input.find(':'));
        if (tmp != "Content-Length" && tmp != "Host" && tmp != "Cookie")
        {
            string value;
            value = input.substr(input.find(':') + 2);
            trim(value, "\n\t\r ");
            this->_head[tmp] = value;
        }
        else if (tmp == "Content-Length")
        {
            // line >> tmp1
            tmp1 = input.substr(input.find(':') + 2);
        }
        else if (tmp == "Host")
        {
            _host = input.substr(input.find(':') + 2);
            if (this->_host.find(':') != string::npos)
            {
                this->_port = this->_host.substr(this->_host.find(':') + 1);
                this->_host = this->_host.substr(0, this->_host.find(':'));
            }
        }
        else if (tmp == "Cookie")   
            _cookie = input.substr(input.find('=') + 1);
    }
    if (this->_host.empty())
        throw BadRequestException();
    stringstream ss(tmp1);
    ss >> this->_ContentLength;
    if (((tmp1.empty() && _head["Transfer-Encoding"].empty()) || tmp1[0] == '-') && _method == "POST")
        throw BadRequestException();
    int pos = Header.find("\r\n\r\n");
    this->restHeader = Header.substr(pos + 4);
}

void RedirectionRequest(Request &req)
{
    if (req.getLocation()->getRetur().empty())
        return;
    map<int, string>::const_iterator it = req.getLocation()->getRetur().begin();
    if ((it->first >= 300 && it->first <= 308))
    {
        req.setRedirectionStatus();
        throw BadRequestException();
    }
}

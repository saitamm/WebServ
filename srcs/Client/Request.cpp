#include "../../Includes/Request.hpp"

Request::Request()
{
}
Request::~Request() {}

// getters && setters

string &Request::getHost(void) { return (_host); }
string &Request::getMethod(void) { return (_method); }
string &Request::getUri(void) { return (_url[0]); }
string &Request::getQuery(void)
{
    if (_url.size() == 2)
        return (_url[1]);
    throw BadRequestException();
}
string &Request::getCtype(void) { return (_head["Content-Type"]); }
Location *Request::getLocation(void) { return (_locat); }
string &Request::getHeadvalue(string key) { return (_head[key]); }
unsigned long long &Request::getContentLength(void) { return (_ContentLength); }
ConfigFile &Request::getConfigFile(void) { return (_serv); }
bool Request::getRedirectionStatus(void) const { return _redir; }
string &Request::getrestHeader(void) { return (restHeader); }

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
    while (line >> tmp && tmp.find("boundary") == string::npos)
    {
        if (tmp != "Content-Length:" && tmp != "Host:")
        {

            trim(tmp, ":");
            string value;
            line >> value;
            trim(value, "\n\t\r ");
            this->_head[tmp] = value;
        }
        else if (tmp == "Content-Length:")
        {
            line >> tmp1;
        }
        else if (tmp == "Host:")
        {
            line >> this->_host;
            if (tmp.find(':') == string::npos)
                throw BadRequestException();
            if (this->_host.find(':') != string::npos)
                this->_host = this->_host.substr(0, this->_host.find(':'));
        }
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
    std::map<int, string>::const_iterator it = req.getLocation()->getRetur().begin();
    if ((it->first >= 300 && it->first <= 308))
    {
        req.setRedirectionStatus();
        throw BadRequestException();
    }
}

#include "../../Includes/Request.hpp"

Request::Request()
{
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
    cout << "header parsed successfully." << Header << endl;

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
    while (getline(line, input) && input != "\r\n")
    {
        tmp = input.substr(0, input.find(':'));
        cout << "header line  = " << tmp <<endl;
        if (tmp != "Content-Length:" && tmp != "Host:")
        {
            cout << "Header key: " << tmp << endl;
            string value;
            value = input.substr(input.find(':') + 2);
            trim(value, "\n\t\r ");
            this->_head[tmp] = value;
        }
        else if (tmp == "Content-Length:")
        {
            // line >> tmp1
            tmp1 = input.substr(input.find(':') + 2);
        }
        else if (tmp == "Host:")
        {
            _host = input.substr(input.find(':') + 2);
            // if (tmp.find(':') == string::npos)
            //     throw BadRequestException();
            if (this->_host.find(':') != string::npos)
            {
                this->_port = this->_host.substr(this->_host.find(':') + 1);
                this->_host = this->_host.substr(0, this->_host.find(':'));
            }
        }
        else if (tmp == "Cookie")
        {
            cout << "-----------------\n";
            line >> this->_cookie;
            _cookie = _cookie.substr(_cookie.find('=') + 1);
            cout << "Cookie: " << _cookie << endl;
            if (this->_cookie.find('=') == string::npos)
                throw BadRequestException();
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
    for (map<string, string>::iterator it = this->_head.begin(); it != this->_head.end(); ++it)
    {
        // if (it->first == "Content-Type")
        // {
        // this->_head["Content-Type"] = it->second;
        cout << "Header: " << it->first << " = " << it->second << endl;
        //     break;
        // }
    }
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

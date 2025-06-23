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
    string tmp1;
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
    if ((tmp1.empty() && _head["Transfer-Encoding"].empty()) || tmp1[0] == '-' || ss.fail())
        throw BadRequestException();
    if (_method == "GET" || _method == "DELETE")
    {
        cout << "soumaaaaaya\n";
        return;
    }
    int pos = tmp.find('-');
    tmp.erase(0, pos);
    string Body;
    unsigned long long byte = 0;
    cout << "tmp = " << tmp <<endl;
    ofstream file;
    while ((bytesRead = recv(clientSocket, buf, sizeof(buf), 0)) > 0)
    {
        Body.append(buf, bytesRead);
        file.write(buf, bytesRead);
        byte+=bytesRead;
        if (byte >= this->_ContentLength || Body.find("\r\n") != string::npos)
        break;
    }
    cout << "Body = \n"
    << Body << endl;
}

// setters

string &Request::getHost(void) { return (_host); }
string &Request::getMethod(void) { return (_method); }
string &Request::getUri(void) { return (_url[0]); }
string &Request::getQuery(void) { return (_url[1]); }
string &Request::getCtype(void) { return (_head["Content-Type"]); }
unsigned long long &Request::getContentLength(void) { return (_ContentLength); }

// getters

void Request::setHost(string host) { _host = host; }
void Request::setMethod(string meth) { _method = meth; }
void Request::setContentLength(unsigned long long Content) { _ContentLength = Content; }

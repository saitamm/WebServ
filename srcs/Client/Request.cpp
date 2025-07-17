#include "../../Includes/Request.hpp"

Request::Request()
{
    _HeadF = false;
    _BodyF = false;
}
Request::~Request() {}
// getters

string &Request::getHost(void) { return (_host); }
string &Request::getMethod(void) { return (_method); }
string &Request::getUri(void) { return (_url[0]); }
string &Request::getQuery(void) { return (_url[1]); }
string &Request::getFilename(void) { return (filename); }
string &Request::getCtype(void) { return (_head["Content-Type"]); }
Location *Request::getLocation(void) { return (_locat); }
string &Request::getHeadvalue(string key) { return (_head[key]); }
unsigned long long &Request::getContentLength(void) { return (_ContentLength); }
ConfigFile &Request::getConfigFile(void) { return (_serv); }

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
    if (((tmp1.empty() && _head["Transfer-Encoding"].empty()) || tmp1[0] == '-' || ss.fail()) && _method == "POST")
        throw BadRequestException();
    int pos = Header.find("\r\n\r\n");
    this->restHeader = Header.substr(pos + 4);
    this->_HeadF = true;
}

void Request::ParseBody(fstream &body, int clientSocket)
{
    char buf[1024];
    size_t bytesRead = 0;
    if (!this->_head["Transfer-Encoding"].empty())
    {
        string del;
        stringstream ss(this->restHeader);
        ss >> del;
        this->restHeader.erase(0, del.size() + 1);
        if (!this->totalReceived)
        {
            this->totalReceived += this->restHeader.size();
            body.write(this->restHeader.c_str(), this->totalReceived);
        }
        // while (1)
        // {
        bytesRead = recv(clientSocket, buf, sizeof(buf) - 1, 0);
        cout << "byteread=" << bytesRead << endl;
        if (bytesRead < 0)
            throw SocketErrorException();
        if (bytesRead == 0)
            throw BadRequestException();
        buf[bytesRead] = '\0';
        body.write(buf, bytesRead);
        body.flush();
        body.clear();
        string gg(buf);
        body.seekg(0);
        std::string buffer((std::istreambuf_iterator<char>(body)),
                           std::istreambuf_iterator<char>());
        cout.flush();
        if (buffer.find("\r\n") != string::npos)
        {
            this->_body = true;
        }
    }
    else
    {
        if (!this->totalReceived)
        {
            this->totalReceived += this->restHeader.size();
            body.write(this->restHeader.c_str(), this->totalReceived);
        }
        string Body;
        cout << "totalreceive = " << this->totalReceived << "   content = " << this->_ContentLength << endl;

        bytesRead = recv(clientSocket, buf, sizeof(buf) - 1, 0);
        if (bytesRead < 0)
            throw SocketErrorException();
        if (bytesRead == 0)
            throw BadRequestException();
        buf[bytesRead] = '\0';
        body.write(buf, bytesRead);
        body.flush();
        this->totalReceived += bytesRead;
        if (this->totalReceived == this->_ContentLength)
            this->_BodyF = true;
    }
}
void Request::ParseHttpRequest(string &Header, int clientSocket, ConfigFile &serv, fstream &body)
{
    this->_serv = serv;
    char buf[1024];
    ssize_t bytesRead;
    if (!this->_HeadF)
    {
        this->totalReceived = 0;
        if ((bytesRead = recv(clientSocket, buf, sizeof(buf), 0)) > 0)
            Header.append(buf, bytesRead);
        if (Header.find("\r\n\r\n") != std::string::npos)
        {
            this->ParseHeader(Header);
            // matching location
            this->_locat = matchLocation(this->_url[0], serv.getLocations());
            if (!this->_locat)
            {
                cout << "No matching location found for URI: " << this->_url[0] << endl;
                throw BadRequestException();
            }
        }
    }
    if (this->_HeadF && !this->_BodyF)
    {
        if (_method == "GET" || _method == "DELETE")
        {
            return;
        }
        this->ParseBody(body, clientSocket);
    }
}

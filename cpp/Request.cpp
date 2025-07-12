#include "../hpp/Request.hpp"
#include "../hpp/Client.hpp"

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
Location *matchLocation(const std::string &uri, const std::vector<Location> &locations)
{
    Location *bestMatch = NULL;
    size_t maxMatchLength = 0;
    for (size_t i = 0; i < locations.size(); ++i)
    {
        std::string path = locations[i].getPath();
        if (uri.find(path) != string::npos && path.length() > maxMatchLength)
        {
            bestMatch = new Location(locations[i]);
            maxMatchLength = path.length();
        }
    }
    return bestMatch;
}

// void Request::ParseRequest(int clientSocket, ConfigFile &serv)
// {

// else
// {
//
// }

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

// test

void Request::ParseHttpRequest(string &Header, int clientSocket, ConfigFile &serv, fstream &body)
{
    this->_serv = serv;
    char buf[1024];
    ssize_t bytesRead;
    string tmp1;
    if (!this->_HeadF)
    {
        map<int, string> body = this->getConfigFile().getError_page();
        if ((bytesRead = recv(clientSocket, buf, sizeof(buf), 0)) > 0)
            Header.append(buf, bytesRead);
        if (Header.find("\r\n\r\n") != std::string::npos)
        {
            this->_HeadF = true;
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
            this->_locat = matchLocation(this->_url[0], serv.getLocations());
            if (!this->_locat)
            {
                cout << "No matching location found for URI: " << this->_url[0] << endl;
                throw BadRequestException();
            }
            stringstream ss(tmp1);
            ss >> this->_ContentLength;
            if (((tmp1.empty() && _head["Transfer-Encoding"].empty()) || tmp1[0] == '-' || ss.fail()) && _method == "POST")
                throw BadRequestException();
        }
    }
    else
    {
        if (_method == "GET" || _method == "DELETE")
        {
            return;
        }
        // body
        int pos = Header.find("\r\n\r\n");
        string buff = Header.substr(pos + 4);
        size_t totalReceived = 0;
        if (!this->_head["Transfer-Encoding"].empty())
        {
             std::string buffer((std::istreambuf_iterator<char>(body)),
                       std::istreambuf_iterator<char>());
            cout << "buffer = "<< buffer << "end" <<endl;
            while (buffer.find("\r\n\r\n") != string::npos)
            {
                bytesRead = recv(clientSocket, buf, sizeof(buf), 0);
                if (bytesRead < 0)
                    throw SocketErrorException();
                if (bytesRead == 0)
                {
                    throw BadRequestException();
                }
                cout << buf << endl;
                buf[bytesRead] = '\0';
                body.write(buf, bytesRead);
                buffer.append(buf, bytesRead);
            }
        }
        else
        {
            while (totalReceived < this->_ContentLength)
            {
                bytesRead = recv(clientSocket, buf, sizeof(buf), 0);
                if (bytesRead < 0)
                    throw SocketErrorException();
                if (bytesRead == 0)
                {
                    throw BadRequestException();
                }
                // body << buf ;
                buf[bytesRead] = '\0';
                body.write(buf, bytesRead);
                // Body.append(buf, bytesRead);
                totalReceived += bytesRead;
            }
        }
        this->_BodyF = true;
    }
}

#include "../hpp/Request.hpp"

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

void Request::ParseRequest(map<int, string> &buffers, int clientSocket, ConfigFile &serv)
{
    this->_serv = serv;
    char buf[1024];
    ssize_t bytesRead;
    string tmp1;
    cout << "clientSocket  = " << clientSocket << endl;
    if (!this->_HeadF)
    {
        map<int, string> body = this->getConfigFile().getError_page();
        if ((bytesRead = recv(clientSocket, buf, sizeof(buf), 0)) > 0)
            buffers[clientSocket].append(buf, bytesRead);
        if (buffers[clientSocket].find("\r\n\r\n") != std::string::npos)
        {

            this->_HeadF = true;
            cout << "Header is finished " << this->_HeadF << endl;
            stringstream line(buffers[clientSocket]);
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
        }
    }
    else
    {
        if (_method == "GET" || _method == "DELETE")
        {
            return;
        }
        // body
        stringstream ss(tmp1);
        ss >> this->_ContentLength;
        if ((tmp1.empty() && _head["Transfer-Encoding"].empty()) || tmp1[0] == '-' || ss.fail())
            throw BadRequestException();
        // int pos = tmp.find('-');
        // tmp.erase(0, pos);
        int pos = buffers[clientSocket].find("\r\n\r\n");
        string buff = buffers[clientSocket].substr(pos + 4);
        cout << "rest in header =" << buff << "=" << endl;
        // string Body;
        // if (this->_ContentLength > this->_serv.getMax_size())
        //     throw BadRequestException();
        // unsigned long long totalReceived = 0;
        // srand(time(0));
        // stringstream ll;
        // ll << rand();
        // this->filename = "Body/body_" + ll.str() + ".txt";
        // ofstream file(filename.c_str());
        // while (totalReceived < this->_ContentLength)
        // {
        //     bytesRead = recv(clientSocket, buf, sizeof(buf), 0);
        //     if (bytesRead < 0)
        //         throw SocketErrorException();
        //     if (bytesRead == 0)
        //         throw BadRequestException();
        //     Body.append(buf, bytesRead);
        //     totalReceived += bytesRead;
        // }
        // file << Body;
        // file.close();
        this->_BodyF = true;
    }
}

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

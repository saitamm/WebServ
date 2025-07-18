#include "Includes/Client.hpp"

void printRequest(Request req)
{

    cout << "method = ******" << req.getMethod() << endl;
    cout << "uri = ******" << req.getUri() << endl;
    if (req.getQuery().empty())
        cout << "query = ******" << req.getQuery() << endl;
    cout << "host = ******" << req.getHost() << endl;
    cout << "type = ******" << req.getCtype() << endl;
    string filename = req.getFilename();
    ifstream file(filename.c_str());
    std::string buffer((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    cout << buffer << endl;
}
void printServ(ConfigFile serv)
{
    cout << " name  = " << serv.getName() << endl;
    cout << " host  = " << serv.getHost() << endl;
    cout << " port  = " << serv.getPort() << endl;
    cout << " root  = " << serv.getRoot() << endl;
    cout << " index  = " << serv.getIndex() << endl;
    vector<Location> loc = serv.getLocations();
    for (size_t i = 0; i < loc.size(); i++)
    {
        cout << "location path = " << loc[i].getPath() << endl;
        for (std::set<std::string>::const_iterator it = loc[i].getMethods().begin(); it != loc[i].getMethods().end(); ++it)
        {
            std::cout << "method = " << *it << std::endl;
        }
        cout << "======\n";
    }
    map<int, string> body = serv.getError_page();
    for (map<int, string>::iterator it = body.begin(); it != body.end(); it++)
        cout << "key =" << it->first << "  value = " << it->second << endl;
}

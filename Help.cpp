#include "hpp/Response.hpp"
#include "hpp/Client.hpp"
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

void SendResponse(Response &resp, int clientSocket)
{
    ostringstream response;
    response << "HTTP/1.1 " << resp.getStatus() << " " << resp.getValue(resp.getStatus()) << "\r\n";
    response << "Content-type: " << resp.getType() << "\r\n";
    response << "Content-Length: " << resp.getBody().size() << "\r\n";
    response << "Connection: close\r\n\r\n";
    response << resp.getBody();
    string final_resp = response.str();
    if (send(clientSocket, final_resp.c_str(), final_resp.size(), 0) == -1)
        cerr << "error Send \n";
    else
        cout << "response sended ✅ \n";
}
void SetErrors(Response &resp)
{
    resp.setError(200, "OK");
    resp.setError(204, "No Content");
    resp.setError(403, "Forbidden");
    resp.setError(404, "Not Found");
    resp.setError(405, "Method Not Allowed");
    resp.setError(409, "Conflict");
}

void ServClient(map<int, string> &buffers, int clientSocket, vector<ConfigFile> *servers)
{
    Request req;
    Response resp;
    SetErrors(resp);
    try
    {
        req.ParseRequest(buffers, clientSocket, servers->at(0));
        if (req.getfinishedHead() == true)
        {
            cout << "ccccccc" << endl;
            MakeResponce(req, resp);
        }
    }
    catch (const std::exception &e)
    {
        resp.setRequest(req);
        resp.setSend();
        setErrorBodyStatus(resp, 400);
    }
    if (resp.getSend())
        SendResponse(resp, clientSocket);
}
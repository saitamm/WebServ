#include "../../../Includes/Response.hpp"
#include "../../../Includes/Client.hpp"

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
}
void setStatusCode(Response &resp)
{
    resp.setCode(200, "OK");
    resp.setCode(204, "No Content");
    resp.setCode(403, "Forbidden");
    resp.setCode(404, "Not Found");
    resp.setCode(405, "Method Not Allowed");
    resp.setCode(409, "Conflict");
}
int allowMethod(Location loc, string method)
{
    for (std::set<std::string>::const_iterator it = loc.getMethods().begin(); it != loc.getMethods().end(); ++it)
    {
        if (*it == method)
            return (1);
    }
    return (0);
}

void MakeResponce(Client &client)
{
    // resp.setRequest(req);
    // if (!allowMethod(*req.getLocation(), req.getMethod()))
    // {
    //     setCodeBodyStatus(resp, 405);
    //     resp.setSend();
    //     return;
    // }
    // if (req.getMethod() == "DELETE")
    // {
    //     handleDelete(resp);
    //     resp.setSend();
    //     return;
    // }
    // if (req.getMethod() == "GET")
    // {
    //     handleGet(resp);
    //     resp.setSend();
    //     return;
    // }
    // if (req.getMethod() == "POST" && req.getfinishedBody())
    // {
    //     handlePost(resp);
    //     resp.setSend();
    //     return;
    // }
}

void handleClientRequest(map<int, Client *> &clients, int clientSocket, vector<ConfigFile> *servers)
{
    // setStatusCode(*clients[clientSocket]->getResp());
    try
    {
        //here we have to match server
        clients[clientSocket]->getRequest()->ParseHttpRequest(clients[clientSocket]->getbuff(), clientSocket, servers->at(0), clients[clientSocket]->getbody());
        if (clients[clientSocket]->getRequest()->getfinishedHead() == true)
        {
            MakeResponce(*clients[clientSocket]);
            clients[clientSocket]->getResp()->setSend();
        }
    }
    catch (const std::exception &e)
    {
        clients[clientSocket]->getResp()->setRequest(*clients[clientSocket]->getRequest());
        clients[clientSocket]->getResp()->setSend();
        setCodeBodyStatus(*clients[clientSocket]->getResp(), 400);
        cout << "i am exception \n";
    }
    if (clients[clientSocket]->getResp()->getSend())
        SendResponse(*clients[clientSocket]->getResp(), clientSocket);

}
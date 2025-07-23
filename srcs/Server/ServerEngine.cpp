#include "../../Includes/Client.hpp"
// check this fucntion because the body is not necessery set here
void setCodeStatus(Response &resp, int error)
{
    resp.setStatus(error);
    map<int, string> body = resp.getRequest()->getConfigFile().getError_page();
    // if (body[error][0] == '/')
    //     body[error].erase(0, 1);
    //  default error pages
    if (body[error].empty())
    {
        string defaultErrorPage = resp.getRequest()->getConfigFile().getDefaultErrorPage(error);
        getContentType(defaultErrorPage, resp);
        ifstream file(defaultErrorPage.c_str());
        if (!file.is_open())
        {
            std::cerr << "❌ Failed to open file: " << body[error] << std::endl;
            return;
        }
        std::string buffer((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        resp.setBody(buffer);
        file.close();
    }
    else
    {
        getContentType(body[error], resp);
        ifstream file(body[error].c_str());
        if (!file.is_open())
        {
            std::cerr << "❌ Failed to open file: " << body[error] << std::endl;
            return;
        }
        std::string buffer((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        resp.setBody(buffer);
        file.close();
    }
}

void SendResponse(Response &resp, int clientSocket)
{
    ostringstream response;
    response << "HTTP/1.1 " << resp.getStatus() << " " << resp.getStatusValue(resp.getStatus()) << "\r\n";
    response << "Content-type: " << resp.getType() << "\r\n";
    response << "Content-Length: " << resp.getBody().size() << "\r\n\r\n";
    // response << "\r\n\r\n";
    response << resp.getBody();
    // cout << "Body ==="<<resp.getBody() << "===\n";
    string final_resp = response.str();
    cout << "Response to be sent: \n" << final_resp << endl;
    if (send(clientSocket, final_resp.c_str(), final_resp.size(), 0) == -1)
        cerr << "error Send \n";
    else
        cout << "Response sent successfully to client socket: " << clientSocket << endl;
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

void handleClientRequest(map<int, Client *> &clients, int clientSocket, vector<ConfigFile> *servers)
{
    try
    {
        // here we have to match
        clients[clientSocket]->getResp()->initStatusCode();
        cout << "==========="<<clients[clientSocket]->getResp()->getStatusValue(200)<<endl;
        clients[clientSocket]->getRequest()->ParseHttpRequest(clients[clientSocket]->getbuff(), clientSocket, servers->at(0), clients[clientSocket]->getbody());
        if (clients[clientSocket]->getRequest()->getfinishedHead() == true)
        {
            clients[clientSocket]->buildResponse();
            clients[clientSocket]->getResp()->setSend();
        }
    }
    catch (const std::exception &e)
    {
        clients[clientSocket]->getResp()->setRequest(*clients[clientSocket]->getRequest());
        clients[clientSocket]->getResp()->setSend();
        setCodeStatus(*clients[clientSocket]->getResp(), 400);
        cout << "i am exception \n";
    }
    if (clients[clientSocket]->getResp()->getSend())
    {
        cout << "::::::::::::::::::::::::::::::::::::::::::\n";
        SendResponse(*clients[clientSocket]->getResp(), clientSocket);
    }
}

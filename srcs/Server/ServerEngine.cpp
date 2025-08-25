#include "../../Includes/Client.hpp"
// check this fucntion because the body is not necessery set here
void setCodeStatus(Response &resp, int error)
{
    std::cout << "resp ptr = " << &resp << std::endl;
    std::cout << "request ptr = " << resp.getRequest() << std::endl;

    if (resp.getRequest()->getRedirectionStatus())
    {
        cout << "HEREEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEee\n";
        cout << "---------------------------------" << resp.getRequest()->getRedirectionStatus() << "\n";
        resp.setStatus(resp.getRequest()->getLocation()->getRetur().begin()->first);
        return;
    }
    resp.setStatus(error);
    map<int, string> body = resp.getRequest()->getConfigFile().getError_page();
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
        resp.setBodyResp(buffer);
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
        resp.setBodyResp(buffer);
        file.close();
    }
}

void SendResponse(Response &resp, int clientSocket)
{
    ostringstream response;
    response << "HTTP/1.1 " << resp.getStatus() << " " << resp.getStatusValue(resp.getStatus()) << "\r\n";
    response << "Content-type: " << resp.getType() << "\r\n";
    if (resp.getRequest()->getRedirectionStatus())
    {
        Location *loc = resp.getRequest()->getLocation();
        if (loc)
        {
            const map<int, string> &returMap = loc->getRetur();
            if (!returMap.empty())
                response << "Location: " << returMap.begin()->second << "\r\n";
        }
    }
    // cout << "============" << resp.getRequest()->getLocation()->getPath();
    response << "Content-Length: " << resp.getBody().size() << "\r\n\r\n";
    response << resp.getBody();
    string final_resp = response.str();
    cout << "****** Response *******\n"
         << final_resp << endl;
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

void handleClientRequest(map<int, Client *> &clients, int clientSocket, vector<ConfigFile> *servers, int epollFd, map<int, CgiProcess*> &cgis)
{
    try
    {
        clients[clientSocket]->getResp()->initStatusCode();
        clients[clientSocket]->ParseHttpRequest(*clients[clientSocket], clientSocket, servers->at(0));
        if (clients[clientSocket]->getStatus() == Body || clients[clientSocket]->getStatus() == Processing)
            clients[clientSocket]->buildResponse(clientSocket, epollFd, cgis);
    }
    catch (const std::exception &e)
    {
        clients[clientSocket]->getResp()->setRequest(*clients[clientSocket]->getRequest());
        clients[clientSocket]->setStatus(Sending);
        if (!clients[clientSocket]->getRequest()->getRedirectionStatus())
            setCodeStatus(*clients[clientSocket]->getResp(), 400);
        else
            setCodeStatus(*clients[clientSocket]->getResp(), 0);
    }
    if (clients[clientSocket]->getStatus() == Sending)
    {
        cout << "i am hereeeeeeeeeeeeeeeeeeeeeeeeeeeee\n";

        SendResponse(*clients[clientSocket]->getResp(), clientSocket);
        clients[clientSocket]->setStatus(Finished);
        if (clients[clientSocket]->getStatus() == Finished)
        {
            delete clients[clientSocket];
            clients.erase(clientSocket);
            close(clientSocket);
        }
    }
}

#include "../../Includes/Client.hpp"
// check this fucntion because the body is not necessery set here
void setCodeStatus(Response &resp, int error)
{
    if (resp.getRequest()->getRedirectionStatus())
    {
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
            cerr << "❌ Failed to open file: " << body[error] << endl;
            return;
        }
        string buffer((istreambuf_iterator<char>(file)),
                      istreambuf_iterator<char>());
        resp.setBodyResp(buffer);
        file.close();
    }
    else
    {
        getContentType(body[error], resp);
        ifstream file(body[error].c_str());
        if (!file.is_open())
        {
            cerr << "❌ Failed to open file: " << body[error] << endl;
            return;
        }
        string buffer((istreambuf_iterator<char>(file)),
                      istreambuf_iterator<char>());
        resp.setBodyResp(buffer);
        file.close();
    }
}
void generateUser(Response &resp)
{
    if (resp.getRequest()->getCookie().empty())
    {
        string Id;
        stringstream ss;
        ss << rand();
        Id = ss.str();
        resp.setSessionId(Id);
    }
    else
    {
        resp.setSessionId(resp.getRequest()->getCookie());
    }
}
void SendResponse(Response &resp, int clientSocket)
{
    if (resp.getResponseStatus() == Nonchunked)
    {
        ostringstream response;
        response << "HTTP/1.1 " << resp.getStatus() << " " << resp.getStatusValue(resp.getStatus()) << "\r\n";
        response << "Content-type: " << resp.getType() << "\r\n";
        if (resp.getRequest()->getRedirectionStatus())
        {
            response << "Location: " << resp.getRequest()->getLocation()->getRetur().begin()->second << "\r\n";
        }
        generateUser(resp);
        response << "Set-Cookie: user=" << resp.getSessionId() << "\r\n";
        response << "Content-Length: " << resp.getBody().size() << "\r\n\r\n";
        response << resp.getBody();

        // cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
        // cout << response.str() << endl;
        // cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
        // cout << "Response to be sent:\n" << final_resp << endl;
        int bytesend;
        if ((bytesend = send(clientSocket, response.str().c_str(), response.str().size(), 0)) == -1)
            cerr << "error Send \n";
        else
            cout << "Response sent successfully to client socket: " << bytesend << endl;
        resp.setResponseStatus(Finish);
    }
    else
    {
        if (resp.getResponseStatus() == First)
        {
            ostringstream response;
            response << "HTTP/1.1 " << resp.getStatus() << " " << resp.getStatusValue(resp.getStatus()) << "\r\n";
            response << "Content-type: " << resp.getType() << "\r\n";

            generateUser(resp);
            response << "Set-Cookie: user=" << resp.getSessionId() << "\r\n";
            response << "Connection: keep-alive\r\n";
            response << "Transfer-Encoding: chunked\r\n\r\n";
            response << hex << resp.getBody().size() << "\r\n";
            response << resp.getBody() << "\r\n";
            // cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
            // cout << response.str() << endl;
            // cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
            int bytesend;
            if ((bytesend = send(clientSocket, response.str().c_str(), response.str().size(), 0)) == -1)
                cerr << "error Send \n";
            else
                cout << "First chunk sent successfully to client socket: " << bytesend << endl;
            resp.setResponseStatus(chunked); // Set to chunked to continue reading
        }
        else if (resp.getResponseStatus() == chunked)
        {
            ostringstream response;
            response << hex << resp.getBody().size() << "\r\n";
            response << resp.getBody() << "\r\n";
            int bytesend;
            if ((bytesend = send(clientSocket, response.str().c_str(), response.str().size(), 0)) == -1)
                cerr << "error Send \n";
            cout << "Chunk sent successfully to client socket: " << bytesend << endl;
        }
        else if (resp.getResponseStatus() == Last)
        {
            ostringstream response;
            response << "0\r\n\r\n";
            int bytesend;
            if ((bytesend = send(clientSocket, response.str().c_str(), response.str().size(), 0)) == -1)
                cerr << "error Send \n";
            else
                cout << "Last chunk sent successfully to client socket: " << bytesend << endl;
            resp.setResponseStatus(Finish);
        }
    }
}
int allowMethod(Location loc, string method)
{
    for (set<string>::const_iterator it = loc.getMethods().begin(); it != loc.getMethods().end(); ++it)
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
        clients[clientSocket]->getResp()->initStatusCode();
        clients[clientSocket]->ParseHttpRequest(*clients[clientSocket], clientSocket, *servers);
        if (clients[clientSocket]->getStatus() == Processing || clients[clientSocket]->getStatus() == Sending)
        {
            clients[clientSocket]->buildResponse();
        }
    }
    catch (const exception &e)
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
        SendResponse(*clients[clientSocket]->getResp(), clientSocket);
        clients[clientSocket]->setNewSessionId(clients[clientSocket]->getResp()->getSessionId());
        if (clients[clientSocket]->getResp()->getResponseStatus() == Finish)
            clients[clientSocket]->setStatus(Finished);
        if (clients[clientSocket]->getStatus() == Finished)
        {

            delete clients[clientSocket];
            clients.erase(clientSocket);
            close(clientSocket);
        }
    }
}

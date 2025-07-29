#include "../../../Includes/Response.hpp"

int SupportUpload(Response &resp)
{
    if (resp.getRequest()->getLocation()->getUp_store().empty())
        return (1);
    return (0);
}

int handlePost(Response &resp, int clientSocket)
{
    if (SupportUpload(resp))
    {
        setCodeStatus(resp, 403);
        return (1);
    }
    char buf[1024];
    int bytesRead = 0;
    if (!resp.getTotalReceived())
    {
        resp.setTotalReceived(resp.getRequest()->getrestHeader().size());
        resp.getFile().write(resp.getRequest()->getrestHeader().c_str(), resp.getTotalReceived());
    }
    string Body;
    bytesRead = recv(clientSocket, buf, sizeof(buf) - 1, 0);
    if (bytesRead < 0)
        throw SocketErrorException();
    if (bytesRead == 0)
        throw BadRequestException();
    // buf[bytesRead] = '\0';
    cout << bytesRead << endl;
    resp.getFile().write(buf, bytesRead);
    // resp.getFile().flush();
    resp.setTotalReceived(bytesRead);
    cout << "Total receive = " << resp.getTotalReceived() << "  ContentLength  = " << resp.getRequest()->getContentLength() << endl;
    if (resp.getTotalReceived() == resp.getRequest()->getContentLength())
    {
        if (!resp.getFile().is_open())
            resp.getFile().close();
        setCodeStatus(resp, 200);
        return (1);
    }
    return (0);
}
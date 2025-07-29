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
    if (resp.getRequest()->getHeadvalue("Transfer-Encoding").empty())
    {

        if (!resp.getTotalReceived())
        {
            resp.setTotalReceived(resp.getRequest()->getrestHeader().size());
            resp.getFile().write(resp.getRequest()->getrestHeader().c_str(), resp.getTotalReceived());
        }
        string Body;
        bytesRead = recv(clientSocket, buf, sizeof(buf), 0);
        if (bytesRead < 0)
            throw SocketErrorException();
        if (bytesRead == 0)
            throw BadRequestException();
        resp.getFile().write(buf, bytesRead);
        resp.getFile().flush();
        resp.setTotalReceived(bytesRead);

        if (resp.getTotalReceived() == resp.getRequest()->getContentLength())
        {
            if (resp.getFile().is_open())
                resp.getFile().close();
            setCodeStatus(resp, 200);
            return (1);
        }
    }
    else
    {
        if (!resp.getTotalReceived())
        {
            resp.setTotalReceived(resp.getRequest()->getrestHeader().size());
            cout << resp.getRequest()->getrestHeader() << endl;
            stringstream ll(resp.getRequest()->getrestHeader());
            string l;
            ll >> l;
            unsigned int decimal;
            std::stringstream ss;

            ss << std::hex << l;
            ss >> decimal;
            cout <<  "---------------this is my size----------------" << decimal << endl;
            resp.getFile().write(resp.getRequest()->getrestHeader().c_str(), resp.getTotalReceived());
        }
        string Body;
        bytesRead = recv(clientSocket, buf, sizeof(buf) - 1, 0);
        if (bytesRead < 0)
            throw SocketErrorException();
        if (bytesRead == 0)
            throw BadRequestException();
        resp.getFile().write(buf, bytesRead);
        resp.getFile().flush();
        resp.setTotalReceived(bytesRead);
        buf[bytesRead] = '\0';
        cout << buf << endl;
        if (resp.getTotalReceived() >= resp.getRequest()->getContentLength())
        {
            cout << "----this is what i received----"<< resp.getTotalReceived() <<endl; 
            if (resp.getFile().is_open())
                resp.getFile().close();
            setCodeStatus(resp, 200);
            return (1);
        }
    }
    return (0);
}
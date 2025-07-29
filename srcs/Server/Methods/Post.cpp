#include "../../../Includes/Response.hpp"

int SupportUpload(Response &resp)
{
    if (resp.getRequest()->getLocation()->getUp_store().empty())
        return (1);
    return (0);
}

unsigned int getSize(int clientSocket)
{
    string line;
    char c;
    while (recv(clientSocket, &c, 1, 0))
    {
        line += c;
        if (line.find("\r\n") != string::npos)
            break;
    }
    cout << "i am in getsize function = " << line << endl;
    return (1);
}

void NonChunkedBody(Response &resp, int clientSocket)
{
    char buf[1024];
    ssize_t bytesRead = 0;
    if (!resp.getTotalReceived())
    {
        resp.setTotalReceived(resp.getRequest()->getrestHeader().size());
        resp.getFile().write(resp.getRequest()->getrestHeader().c_str(), resp.getTotalReceived());
    }
    string Body;
    bytesRead = recv(clientSocket, buf, sizeof(buf), 0);
    if (bytesRead < 0)
        throw SocketErrorException();
    resp.getFile().write(buf, bytesRead);
    resp.getFile().flush();
    resp.setTotalReceived(bytesRead);
}

int ChunkedBody(Response &resp, int clientSocket)
{
    int bytesRead = 0;
    unsigned int BufferSize;
    if (!resp.getTotalReceived())
    {
        stringstream ll(resp.getRequest()->getrestHeader());
        string l;
        ll >> l;
        cout << "rest Header =" << ll.str()<<endl;
        std::stringstream ss;
        ss << std::hex << l;
        ss >> BufferSize;
        string line;
        line = ll.str();
        line.erase(0, l.size() + 2);
        vector<string> splited;
        cout << "this is the buffer =" << BufferSize <<endl;
        cout << "this is the Total  = "<< resp.getRequest()->getContentLength() <<endl;
        return(1);
        // split(line, "\r\n", splited);
        // for(int i = 0;i<(int)splited.size();i++)
        //     cout << "----" << splited[i] <<endl;
        // resp.getFile().write(line.c_str(), line.size());
        // resp.setTotalReceived(line.size());
        // cout << "this is my buffer " << BufferSize << "   and  " << resp.getTotalReceived() << endl;
        if (BufferSize > resp.getTotalReceived())
        {

            char buf[BufferSize - resp.getTotalReceived()];
            bytesRead = recv(clientSocket, buf, sizeof(buf), 0);
            if (bytesRead < 0)
                throw SocketErrorException();
            cout << "----------------------------------------" << endl;
            if (bytesRead == 0)
                throw BadRequestException();
            resp.getFile().write(buf, bytesRead);
            resp.getFile().flush();
            resp.setTotalReceived(bytesRead);
            string end(buf);
            BufferSize = getSize(clientSocket);
            if (end.find("0\r\n"))
            {
        
            }
        }
        // else
        // {
        //     if (resp.getFile().is_open())
        //         resp.getFile().close();
        //     setCodeStatus(resp, 200);
        //     return (1);
        // }
    }
    return (1);
}
int handlePost(Response &resp, int clientSocket)
{
    if (SupportUpload(resp))
    {
        setCodeStatus(resp, 403);
        return (1);
    }
    if (resp.getRequest()->getHeadvalue("Transfer-Encoding").empty())
    {
        NonChunkedBody(resp, clientSocket);
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
        if (ChunkedBody(resp, clientSocket))
        {
            if (resp.getFile().is_open())
                resp.getFile().close();
            setCodeStatus(resp, 200);
            return (1);
        }
    }
    return (0);
}
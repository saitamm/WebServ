#include "hpp/Request.hpp"
#include "hpp/Response.hpp"

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
}

void SendResponse(Response &resp, int clientSocket)
{
      ostringstream response;
    response << "HTTP/1.1 "<< resp.getStatus() << " " << resp.getValue(resp.getStatus()) <<"\r\n";
    response << "Content-type: " << resp.getType() << "\r\n";
    response << "Content-Length: " << resp.getBody().size() << "\r\n";
    response << "Connection: close\r\n\r\n";
    response << resp.getBody();
    string final_resp = response.str();
    cout << final_resp << endl;
     
     if (send(clientSocket, final_resp.c_str(), final_resp.size(), 0) == -1)
          cerr << "error Send \n";
     else
          cout << "response sended ✅ \n";
}

int printErr(const string &err)
{
     cerr << err << endl;
     return 1; 
}

void setNonBlocking(int fd)
{
     int flags = fcntl(fd, F_GETFL, 0);
     fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main(int ac, char **av)
{
     if (ac != 2)
          return(printErr("ERROR: ./Webserv <file.conf>"));
     ConfigFile config;
     vector<ConfigFile> *servers;
     try
     {
          servers = config.ParseConfigFile(av[1]);
          int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
          if (serverSocket == -1)
               return(printErr("Socket creation failed!"));
          setNonBlocking(serverSocket);
          int opt = 1;
          setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
          sockaddr_in serverAddr;
          memset(&serverAddr, 0, sizeof(serverAddr));
          serverAddr.sin_family = AF_INET;
          serverAddr.sin_addr.s_addr = INADDR_ANY;
          serverAddr.sin_port = htons(servers->at(0).getPort());
          if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
               return(printErr("bind failed, maybe port is busy!"));
          listen(serverSocket, SOMAXCONN);
          
          int epollFd = epoll_create1(0);
          epoll_event event;
          memset(&event, 0, sizeof(event));
          event.data.fd = serverSocket;
          event.events = EPOLLIN;
          epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &event);

          const int MAX_EVENTS = 1000;
          epoll_event events[MAX_EVENTS];
          while(1)
          {
               int n = epoll_wait(epollFd, events, MAX_EVENTS, -1);
               for(int i = 0; i < n; ++i)
               {
                    if (events[i].data.fd == serverSocket)
                    {
                         int clientSocket = accept(serverSocket, NULL, NULL);
                         setNonBlocking(clientSocket);
                         epoll_event clientEvent;
                         memset(&clientEvent, 0, sizeof(clientEvent));
                         clientEvent.data.fd = clientSocket;
                         clientEvent.events = EPOLLIN | EPOLLET;
                         epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &clientEvent);
                         cout << " New client connected: "<< clientSocket << endl;
                    }
                    else
                    {
                         Request req;
                         Response resp;
                         try
                         {
                              resp.setError(200, "OK");
                              resp.setError(204, "No Content");
                              resp.setError(403, "Forbidden");
                              resp.setError(404, "Not Found");
                              resp.setError(405, "Method Not Allowed");
                              resp.setError(409, "Conflict");
                              req.ParseRequest(events[i].data.fd, servers->at(0));
                              MakeResponce(req, resp);
                         }
                         catch (const std::exception &e)
                         {
                              cout << "---------------\n";
                              resp.setStatus(400);
                              map<int, string> body = servers->at(0).getError_page();
                              if (body[400][0] == '/')
                                   body[400].erase(0, 1);
                              fstream file(body[400].c_str());
                              if (!file.is_open())
                              {
                                   std::cerr << "❌ Failed to open file: " << body[400] << std::endl;
                              }
                              std::string buffer((std::istreambuf_iterator<char>(file)),
                                             std::istreambuf_iterator<char>());
                              resp.setBody(buffer);
                              
                         }
                         SendResponse(resp, events[i].data.fd);
                         close(events[i].data.fd);  
                         
                    }
               }
          }
          close(serverSocket);
     }
     catch(exception& e)
     {
          cout << e.what() <<endl;
     }

}


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
int main(int ac, char **av)
{
     if (ac != 2)
     {
          cout << "Error: ./Webserv <file.conf> \n";
          return 1;
     }
     ConfigFile config;
     vector<ConfigFile> *servers;
     try
     {

          servers = config.ParseConfigFile(av[1]);
          cout << "--------------------Server-----------------\n";
          printServ(servers->at(0));
          int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
          if (serverSocket == -1)
          {
               std::cerr << "Socket creation failed!\n";
               return 1;
          }
          sockaddr_in serverAddr;
          serverAddr.sin_family = AF_INET;
          serverAddr.sin_port = htons(servers->at(0).getPort());
          serverAddr.sin_addr.s_addr = INADDR_ANY;
          int opt = 1;
          setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
          if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
          {
               std::cerr << "Bind failed! Maybe port is busy.\n";
               return 1;
          }
          listen(serverSocket, 5);
          int clientSocket;
          clientSocket = accept(serverSocket, NULL, NULL);
          cout << "--------------------Request-----------------\n\n";
          Request req;
          req.ParseRequest(clientSocket, servers->at(0));
          printRequest(req);
          Response resp;
          cout << "--------------------Response-----------------\n";
          MakeResponce(req, resp);
          stringstream ss;
          ss << resp.getBody().size();
          std::string response =
              "HTTP/1.1 " + resp.getStatus() + " " + resp.getValue(resp.getStatus()) + "\r\n"
                                                                                       "Content-Type: text/html\r\n"
                                                                                       "Content-Length: " +
              ss.str() +
              "\r\n"
              "Connection: close\r\n"
              "\r\n" +
              resp.getBody();
          if (send(clientSocket, response.c_str(), response.size(), 0) == -1)
               cerr << "error Send \n";
          close(clientSocket);
          close(clientSocket);
     }
     catch (exception &e)
     {
          cout << e.what() << endl;
     }
     return 0;
}

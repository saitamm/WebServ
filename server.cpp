#include "hpp/Request.hpp"
#include "hpp/Response.hpp"
void printRequest(Request req);
void printServ(ConfigFile serv);
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
          close(clientSocket);
          close(clientSocket);
     }
     catch (exception &e)
     {
          cout << e.what() << endl;
     }
     return 0;
}

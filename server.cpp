#include "hpp/Request.hpp"
#include "hpp/Response.hpp"

// void printRequest9(Request req)
// {
//      cout <<
// }
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
          for (std::set<std::string>::const_iterator it = loc[i].getMethods().begin(); it != loc[i].getMethods().end(); ++it) {
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
          cout << "--------------------Server-----------------\n";
          // cout << servers->at(0).getName() << endl;
          int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
          if (serverSocket == -1)
          {
               std::cerr << "Socket creation failed!\n";
               return 1;
          }

          sockaddr_in serverAddr;
          serverAddr.sin_family = AF_INET;
          // stringstream ss(servers->at(0).getPort());
          // int value;
          // ss >> value;
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
          Request request;
          int clientSocket;
          try
          {
               clientSocket = accept(serverSocket, NULL, NULL);
               // cout << "-------------------------------------------------------------\n";
               Request req;
               cout << "--------------------Request-----------------\n\n";
               req.ParseRequest(clientSocket, servers->at(0));
               cout << "Request = :::::::::::::::::::::::::::::::::::::::::::;\n";
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
               cout << "--------------------Request-----------------\n";
               // Response resp;
               // cout << "--------------------Response-----------------\n";
               // MakeResponce(req, resp);
               // string buff = "HTTP/1.1 " + resp.getStatus() + "Not found\r\n" + "Content-Type: text/plain\r\n"
               //                                                                  "Content-Length: 19\r\n"
               //                                                                  "\r\n"
               //                                                                  "Permissioin denied.\n";
               const char *response =
                   "HTTP/1.1 200 OK\r\n"
                   "Content-Type: text/plain\r\n"
                   "Content-Length: 27\r\n"
                   "Connection: close\r\n"
                   "\r\n"
                   "File deleted successfully.\n";
               cout << " Res = " << response << endl;
               ssize_t bytes_sent = send(clientSocket, response, strlen(response), 0);
               if (bytes_sent == -1)
               {
                    perror("send failed");
               }
               else
               {
                    std::cout << "Sent " << bytes_sent << " bytes to client." << std::endl;
               }
               cout << "--------------------Response-----------------\n";
               close(clientSocket);
          }
          catch (const std::exception &e)
          {
               std::cerr << "::::::::" << e.what() << '\n';
          }

          close(clientSocket);
     }
     catch (exception &e)
     {
          cout << e.what() << endl;
     }
     return 0;
}

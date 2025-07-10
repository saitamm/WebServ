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
     map<int, string> body = serv.getError_page();
     for (map<int, string>::iterator it = body.begin(); it != body.end(); it++)
          cout << "key =" << it->first << "  value = " << it->second << endl;
}

void SendResponse(Response &resp, int clientSocket)
{
     ostringstream response;
     response << "HTTP/1.1 " << resp.getStatus() << " " << resp.getValue(resp.getStatus()) << "\r\n";
     response << "Content-type: " << resp.getType() << "\r\n";
     response << "Content-Length: " << resp.getBody().size() << "\r\n";
     response << "Connection: close\r\n\r\n";
     response << resp.getBody();
     string final_resp = response.str();
     if (send(clientSocket, final_resp.c_str(), final_resp.size(), 0) == -1)
          cerr << "error Send \n";
     else
          cout << "response sended ✅ \n";
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
          while (true)
          {
               clientSocket = accept(serverSocket, NULL, NULL);
               cout << "--------------------Request-----------------\n\n";
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
                    req.ParseRequest(clientSocket, servers->at(0));
                    MakeResponce(req, resp);
               }
               catch (const std::exception &e)
               {
                    resp.setRequest(req);
                    setErrorBodyStatus(resp, 400);
               }
               SendResponse(resp, clientSocket);
          }
          close(clientSocket);
          close(serverSocket);
     }
     catch (exception &e)
     {
          cout << e.what() << endl;
     }
     return 0;
}

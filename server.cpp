#include "hpp/Request.hpp"
#include "hpp/Response.hpp"

// void printRequest9(Request req)
// {
//      cout <<
// }
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
          cout << servers->at(0).getName() << endl;
          int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
          if (serverSocket == -1)
          {
               std::cerr << "Socket creation failed!\n";
               return 1;
          }

          sockaddr_in serverAddr;
          serverAddr.sin_family = AF_INET;
          stringstream ss(servers->at(0).getPort());
          int value;
          ss >> value;
          serverAddr.sin_port = htons(value);
          serverAddr.sin_addr.s_addr = INADDR_ANY;
          int opt = 1;
          setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

          if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
          {
               std::cerr << "Bind failed! Maybe port is busy.\n";
               return 1;
          }
          listen(serverSocket, 5);
          std::cout << "Server is running on http://localhost:8080/\n";
          Request request;
          int clientSocket;
          try
          {
               clientSocket = accept(serverSocket, NULL, NULL);
               cout << "-------------------------------------------------------------\n";
               Request req;
               req.ParseRequest(clientSocket, servers->at(0));
               cout << ":::::::::::::::::::::::::::::::::::::::::::;\n";
               cout << "******" << req.getMethod() << endl;
               cout << "******" << req.getUri() << endl;
               if (req.getQuery().empty())
               cout << "******" << req.getQuery() << endl;
               cout << "******" << req.getHost() << endl;
               cout << "******" << req.getCtype() << endl;
               string filename = req.getFilename();
               ifstream file(filename.c_str());
               std::string buffer((std::istreambuf_iterator<char>(file)),
               std::istreambuf_iterator<char>());
               cout << buffer << endl;
               cout << "-------------------------------------------------------------\n";
               Response res;
               res.MakeResponce(req);   


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

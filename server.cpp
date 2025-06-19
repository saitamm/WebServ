#include "Request.hpp"
int main()
{
     int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
     if (serverSocket == -1)
     {
          std::cerr << "Socket creation failed!\n";
          return 1;
     }

     sockaddr_in serverAddr;
     serverAddr.sin_family = AF_INET;
     serverAddr.sin_port = htons(8080); // Port 8080
     serverAddr.sin_addr.s_addr = INADDR_ANY;

     // Allow reuse of the port
     int opt = 1;
     setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

     if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
     {
          std::cerr << "Bind failed! Maybe port is busy.\n";
          return 1;
     }

     listen(serverSocket, 5);
     std::cout << "Server is running on http://localhost:8080/\n";

     const char *httpResponse =
         "HTTP/1.1 200 OK\r\n"
         "Content-Type: text/html\r\n"
         "Content-Length: 20\r\n"
         "\r\n"
         "<h1>Hello world</h1>";
          Request request;
          int clientSocket = accept(serverSocket, NULL, NULL);
          if (clientSocket < 0)
          {
               std::cerr << "Accept failed\n";

          }
          else
          {

               char buffer[1024];
               recv(clientSocket, buffer, sizeof(buffer), 0);
               ParseRequest(buffer, request);
               // std::cout << "Request : \n"
               //           << buffer << std::endl;
               send(clientSocket, httpResponse, strlen(httpResponse), 0);
          }
               close(clientSocket);

     return 0;
}

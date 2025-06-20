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
     Request request;
     int clientSocket = accept(serverSocket, NULL, NULL);
     if (clientSocket < 0)
     {
          std::cerr << "Accept failed\n";
     }
     else
     {
          ParseRequest(clientSocket, request);  
     }
     close(clientSocket);

     return 0;
}

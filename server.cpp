#include "hpp/Request.hpp"
#include "hpp/Response.hpp"
#include "hpp/Client.hpp"

int create_server_socket(vector<ConfigFile> *servers)
{
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
     return (serverSocket);
}
void accept_new_connection(int server_socket, fd_set *all_sockets, int *fd_max)
{
     struct sockaddr_in client_addr;
     socklen_t client_len = sizeof(client_addr);
     int new_client_fd = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);

     if (new_client_fd < 0)
     {
          perror("accept");
          return;
     }

     // Add new socket to the set
     FD_SET(new_client_fd, all_sockets);

     // Update the max fd
     if (new_client_fd > *fd_max)
     {
          *fd_max = new_client_fd;
     }

     std::cout << "New client connected: socket fd " << new_client_fd << std::endl;
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
          int server_socket;
          int status;

          // To monitor socket fds:
          fd_set all_sockets; // Set for all sockets connected to server
          fd_set read_fds;    // Temporary set for select()
          int fd_max;         // Highest socket fd
          struct timeval timer;

          // Create server socket
          server_socket = create_server_socket(servers);
          if (server_socket == -1)
          {
               return (1);
          }

          // Listen to port via socket
          printf("[Server] Listening on port %d\n", servers->at(0).getPort());
          status = listen(server_socket, 10);
          if (status != 0)
          {
               fprintf(stderr, "[Server] Listen error: %s\n", strerror(errno));
               return (3);
          }

          // Prepare socket sets for select()
          FD_ZERO(&all_sockets);
          FD_ZERO(&read_fds);
          FD_SET(server_socket, &all_sockets); // Add listener socket to set
          fd_max = server_socket;              // Highest fd is necessarily our socket
          printf("[Server] Set up select fd sets\n");

          while (1)
          { // Main loop
               // Copy all socket set since select() will modify monitored set
               read_fds = all_sockets;
               // 2 second timeout for select()
               timer.tv_sec = 2;
               timer.tv_usec = 0;

               // Monitor sockets ready for reading
               status = select(fd_max + 1, &read_fds, NULL, NULL, &timer);
               if (status == -1)
               {
                    fprintf(stderr, "[Server] Select error: %s\n", strerror(errno));
                    exit(1);
               }
               else if (status == 0)
               {
                    // No socket fd is ready to read
                    printf("[Server] Waiting...\n");
                    continue;
               }

               cerr << "i have " << fd_max <<endl;
               for (int i = 0; i <= fd_max; i++)
               {
                    if (FD_ISSET(i, &read_fds) != 1)
                    {
                         continue;
                    }
                    printf("[%d] Ready for I/O operation\n", i);
                    // Socket is ready to read!
                    if (i == server_socket)
                    {
                         // Socket is our server's listener socket
                         accept_new_connection(server_socket, &all_sockets, &fd_max);
                    }
                    else
                    {
                         // Socket is a client socket, let's read it
                         // read_data_from_socket(i, &all_sockets, fd_max, server_socket);
                    }
               }
     
          return 0;
     }
}
     catch(std::exception &e)
     {
          cerr << e.what() <<endl;
     }     
}
/*
     printf("---- SERVER ----\n\n");

         int server_socket;
         int status;

         // To monitor socket fds:
         fd_set all_sockets; // Set for all sockets connected to server
         fd_set read_fds;    // Temporary set for select()
         int fd_max;         // Highest socket fd
         struct timeval timer;

         // Create server socket
         server_socket = create_server_socket();
         if (server_socket == -1) {
             return (1);
         }

         // Listen to port via socket
         printf("[Server] Listening on port %d\n", PORT);
         status = listen(server_socket, 10);
         if (status != 0) {
             fprintf(stderr, "[Server] Listen error: %s\n", strerror(errno));
             return (3);
         }

         // Prepare socket sets for select()
         FD_ZERO(&all_sockets);
         FD_ZERO(&read_fds);
         FD_SET(server_socket, &all_sockets); // Add listener socket to set
         fd_max = server_socket; // Highest fd is necessarily our socket
         printf("[Server] Set up select fd sets\n");

         while (1) { // Main loop
             // Copy all socket set since select() will modify monitored set
             read_fds = all_sockets;
             // 2 second timeout for select()
             timer.tv_sec = 2;
             timer.tv_usec = 0;

             // Monitor sockets ready for reading
             status = select(fd_max + 1, &read_fds, NULL, NULL, &timer);
             if (status == -1) {
                 fprintf(stderr, "[Server] Select error: %s\n", strerror(errno));
                 exit(1);
             }
             else if (status == 0) {
                 // No socket fd is ready to read
                 printf("[Server] Waiting...\n");
                 continue;
             }

             // Loop over our sockets
             for (int i = 0; i <= fd_max; i++) {
                 if (FD_ISSET(i, &read_fds) != 1) {
                     // Fd i is not a socket to monitor
                     // stop here and continue the loop
                     continue ;
                 }
                 printf("[%d] Ready for I/O operation\n", i);
                 // Socket is ready to read!
                 if (i == server_socket) {
                     // Socket is our server's listener socket
                     accept_new_connection(server_socket, &all_sockets, &fd_max);
                 }
                 else {
                     // Socket is a client socket, let's read it
                     read_data_from_socket(i, &all_sockets, fd_max, server_socket);
                 }
             }
         }*/
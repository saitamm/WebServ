#include "hpp/Request.hpp"
#include "hpp/Response.hpp"
#include "hpp/Client.hpp"

int create_server_socket(vector<ConfigFile> *servers)
{
     int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
     if (serverSocket == -1)
     {
          std::cerr << "Socket creation failed!\n";
          return -1;
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
          return -1;
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
}
int main(int ac, char **av)
{
     if (ac != 2)
     {
          return 1;
     }
     ConfigFile config;
     vector<ConfigFile> *servers;
     try
     {

          servers = config.ParseConfigFile(av[1]);
          int server_socket;
          int status;

          fd_set all_sockets;
          fd_set read_fds;
          int fd_max;
          struct timeval timer;

          server_socket = create_server_socket(servers);
          if (server_socket == -1)
          {
               return (1);
          }

          status = listen(server_socket, 10);
          if (status != 0)
          {
               return (3);
          }

          FD_ZERO(&all_sockets);
          FD_ZERO(&read_fds);
          FD_SET(server_socket, &all_sockets);
          fd_max = server_socket;
          map<int, string> buffers;
          while (1)
          {
               read_fds = all_sockets;
               timer.tv_sec = 2;
               timer.tv_usec = 0;

               status = select(fd_max + 1, &read_fds, NULL, NULL, &timer);
               if (status == -1)
               {
                    exit(1);
               }
               else if (status == 0)
               {
                    continue;
               }

               for (int i = 0; i <= fd_max; i++)
               {
                    if (FD_ISSET(i, &read_fds) != 1)
                    {
                         continue;
                    }
                    if (i == server_socket)
                    {
                         accept_new_connection(server_socket, &all_sockets, &fd_max);
                    }
                    else
                    {
                         ServClient(buffers,i, servers);
                    }
               }
          }
     }
     catch (std::exception &e)
     {
          cerr << e.what() << endl;
     }
     return (0);
}

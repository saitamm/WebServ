#include "Includes/Client.hpp"

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

     FD_SET(new_client_fd, all_sockets);
     if (new_client_fd > *fd_max)
     {
          *fd_max = new_client_fd;
     }
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

int openSocket(vector<ConfigFile> *servers, int epollFd, map<int, ConfigFile> &openedServers)
{
     for (size_t i = 0; i < servers->size(); i++)
     {
          int serverSocket;
          int port = servers->at(i).getPort();
          bool dupPort = false;
          for (map<int, ConfigFile>::iterator it = openedServers.begin(); it != openedServers.end(); it++)
          {
               if (it->second.getPort() == port)
               {
                    dupPort = true;
                    break;
               }
          }
          if (dupPort)
               continue;
          serverSocket = socket(AF_INET, SOCK_STREAM, 0);
          if (serverSocket == -1)
               return (printErr("reation failed!"));
          setNonBlocking(serverSocket);
          int opt = 1;
          setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
          sockaddr_in serverAddr;
          memset(&serverAddr, 0, sizeof(serverAddr));
          serverAddr.sin_family = AF_INET;
          serverAddr.sin_addr.s_addr = INADDR_ANY;
          serverAddr.sin_port = htons(port);
          if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
               return (printErr("bind failed, maybe port is busy!"));
          listen(serverSocket, SOMAXCONN);
          epoll_event event;
          memset(&event, 0, sizeof(event));
          event.data.fd = serverSocket;
          event.events = EPOLLIN; // ready to accept new clients
          epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &event);
          openedServers[serverSocket] = servers->at(i);
     }
     return 0;
}

void sendCleanUp(Response &resp, int epollFd, CgiProcess *proc, std::map<int, Client *> &clients, std::map<int, CgiProcess *> &cgis)
{
     epoll_ctl(epollFd, EPOLL_CTL_DEL, proc->pipeFd, NULL);
     close(proc->pipeFd);

     std::map<int, Client *>::iterator it = clients.find(proc->clientFd);
     if (it != clients.end())
     {
          Client *client = it->second;
          SendResponse(resp, proc->clientFd);
          client->setStatus(Finished);

          close(proc->clientFd);         // Close client socket
          delete client;                 // Safe deletion
          clients.erase(proc->clientFd); // Remove from map
     }

     // Remove CGI process from map
     cgis.erase(proc->pipeFd);
     delete proc;
}

int main(int ac, char **av)
{
     if (ac != 2)
          return (printErr("ERROR: ./Webserv <file.conf>"));
     ConfigFile config;
     vector<ConfigFile> *servers;
     config.initDefaultError();
     map<int, CgiProcess *> cgis;
     try
     {
          map<int, ConfigFile> openedServers;
          servers = config.ParseConfigFile(av[1]);
          int epollFd = epoll_create1(0);
          if (epollFd == -1)
               return (printErr("Failed to create epoll"));
          if (openSocket(servers, epollFd, openedServers))
               return 1;

          const int MAX_EVENTS = 1000;
          epoll_event events[MAX_EVENTS];
          map<int, Client *> clients;
          std::vector<int> clientsToDelete;
          while (1)
          {
               int n = epoll_wait(epollFd, events, MAX_EVENTS, -1);
               for (int i = 0; i < n; ++i)
               {
                    int fd = events[i].data.fd;

                    // New client connection
                    if (openedServers.find(fd) != openedServers.end())
                    {
                         int clientSocket = accept(fd, NULL, NULL);
                         setNonBlocking(clientSocket);

                         epoll_event clientEvent;
                         memset(&clientEvent, 0, sizeof(clientEvent));
                         clientEvent.data.fd = clientSocket;
                         clientEvent.events = EPOLLIN;
                         epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &clientEvent);

                         std::cout << "New client connected on server port " << openedServers[fd].getPort()
                                   << ": socket = " << clientSocket << std::endl;
                         continue;
                    }

                    // CGI output
                    if (cgis.find(fd) != cgis.end())
                    {
                         CgiProcess *proc = cgis[fd];
                         char buffer[1024];
                         ssize_t bytesRead = read(proc->pipeFd, buffer, sizeof(buffer));
                         if (bytesRead > 0)
                              proc->output.write(buffer, bytesRead);

                         int status;
                         pid_t result = waitpid(proc->pid, &status, WNOHANG);

                         if (result == proc->pid || difftime(time(NULL), proc->start) > 5)
                         {
                              Client *client = clients[proc->clientFd];
                              Response &resp = *client->getResp();
                              resp.setRequest(*client->getRequest());

                              if (result == proc->pid)
                              {
                                   if (WIFEXITED(status))
                                   {
                                        int exitCode = WEXITSTATUS(status);
                                        if (exitCode == 0)
                                        {
                                             std::string outStr = proc->output.str();
                                             if (resp.getRequest()->getMethod() == "POST")
                                             {
                                                  resp.getFile().close();
                                                  resp.getFile().open(resp.getFileName().c_str(), ios::out | ios::trunc | ios::binary);
                                                  resp.getFile().write(outStr.data(), outStr.size());
                                                  resp.getFile().flush();
                                                  if (resp.getFile().is_open())
                                                       resp.getFile().close();
                                                  resp.setStatus(200);
                                             }
                                             else
                                             {
                                                  resp.setBodyResp(outStr);
                                                  resp.setStatus(200);
                                                  resp.setType("text/html");
                                             }
                                        }
                                        else
                                        {
                                             std::cerr << "CGI exited with error code " << exitCode << std::endl;
                                             setCodeStatus(resp, 500);
                                        }
                                   }
                                   else if (WIFSIGNALED(status))
                                   {
                                        int sig = WTERMSIG(status);
                                        std::cerr << "CGI killed by signal " << sig << std::endl;
                                        setCodeStatus(resp, 500);
                                   }
                              }
                              else
                              {
                                   std::cerr << "[CGI] Timeout, killing pid=" << proc->pid << std::endl;
                                   kill(proc->pid, SIGKILL);
                                   waitpid(proc->pid, NULL, 0);
                                   setCodeStatus(resp, 500);
                              }

                              // Cleanup and send response safely
                              sendCleanUp(resp, epollFd, proc, clients, cgis);
                         }
                         continue;
                    }

                    // Normal client request
                    if (events[i].events & (EPOLLHUP | EPOLLRDHUP))
                    {
                         std::cerr << "Client disconnected: fd=" << fd << std::endl;
                         close(fd);
                         delete clients[fd];
                         clients.erase(fd);
                         continue;
                    }

                    if (clients.find(fd) == clients.end())
                         clients[fd] = new Client();

                    handleClientRequest(clients, fd, servers, epollFd, cgis);
               }
          }
          for (map<int, ConfigFile>::iterator it = openedServers.begin(); it != openedServers.end(); ++it)
               close(it->first);
     }
     catch (exception &e)
     {
          cout << e.what() << endl;
     }
}

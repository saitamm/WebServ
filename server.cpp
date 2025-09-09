#include "Includes/Client.hpp"
#include <iostream>

int Client::epollFd = -1;

int printErr(const string &err)
{
     cerr << err << endl;
     return 1;
}

void setNonBlocking(int fd)
{
     if (fcntl(fd, F_SETFL, O_NONBLOCK | FD_CLOEXEC) == -1)
          throw runtime_error("fcntl failed to set O_NONBLOCK | FD_CLOEXEC");
}

int openSocket(auto_ptr<vector<ConfigFile> > &servers, int epollFd, map<int, ConfigFile> &openedServers)
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
          cout << "port is :" << servers->at(i).getPort() << endl;
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
          event.events = EPOLLIN;
          epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &event);
          openedServers[serverSocket] = servers->at(i);
     }
     return 0;
}

void connectClient(int fd, map<int, Client *> &clients, int epollFd)

{
     cout << "i am hereeeeeeee\n";
     int clientSocket = accept(fd, NULL, NULL);
     // std::cout << "Opening socket fd=" << clientSocket << std::endl;
     setNonBlocking(clientSocket);
     if (clients.find(clientSocket) == clients.end())
          clients[clientSocket] = new Client();
     clients[clientSocket]->setEpollFd(epollFd);
     clients[clientSocket]->getEvent().data.fd = clientSocket;
     clients[clientSocket]->getEvent().events = EPOLLIN;
     epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &clients[clientSocket]->getEvent());
     cout << "New connection accepted: fd=" << clientSocket << endl;
}

void timeout(map<int, Client *> &clients, int epollFd)
{
     time_t now = time(NULL);
     std::map<int, Client *>::iterator it;
     for (it = clients.begin(); it != clients.end();)
     {
          Client *client = it->second;
          if (difftime(now, client->getLastActivity()) > 5)
          {
               std::cout << "Closing idle socket fd=" << it->first << std::endl;
               epoll_ctl(epollFd, EPOLL_CTL_DEL, it->first, NULL);
               close(it->first);
               delete client;
               clients.erase(it++);
          }
          else
               ++it;
     }
}

void cleaningAfterSignal(map<int, Client *> &clients, int epollFd, map<int, ConfigFile> &openedServers, map<int, CgiProcess *> &cgis)
{
     std::map<int, Client *>::iterator it;
     for (it = clients.begin(); it != clients.end(); ++it)
     {
          std::cout << "Closing socket fd=" << it->first << std::endl;
          epoll_ctl(epollFd, EPOLL_CTL_DEL, it->first, NULL);
          close(it->first);
          delete it->second;
     }
     clients.clear();

     for (std::map<int, CgiProcess *>::iterator cit = cgis.begin(); cit != cgis.end(); ++cit)
     {
          close(cit->first);
          delete cit->second;
     }
     for (std::map<int, ConfigFile>::iterator sit = openedServers.begin(); sit != openedServers.end(); ++sit)
          close(sit->first);

     close(epollFd);
}

static bool running = true;

void signalHandler(int signum)
{
     (void)signum;
     running = false;
}

int main(int ac, char **av)
{
     if (ac != 2)
          return (printErr("ERROR: ./Webserv <file.conf>"));
     signal(SIGINT, signalHandler);
     ConfigFile config;
     std::auto_ptr<std::vector<ConfigFile> > servers;
     config.initDefaultError();
     map<int, CgiProcess *> cgis;
     try
     {
          map<int, ConfigFile> openedServers;
          servers = config.ParseConfigFile(av[1]);

          int epollFd = epoll_create1(0);
          if (epollFd == -1)
               return printErr("Failed to create epoll");
          if (openSocket(servers, epollFd, openedServers))
               return 1;
          const int MAX_EVENTS = 1000;
          epoll_event events[MAX_EVENTS];
          map<int, Client *> clients;

          while (running)
          {
               int n = epoll_wait(epollFd, events, MAX_EVENTS, 1000);
               if (n < 0)
               {
                    if (errno == EINTR)
                         continue;
                    perror("epoll_wait");
                    break;
               }

               for (int i = 0; i < n; ++i)
               {
                    int fd = events[i].data.fd;

                    if (openedServers.find(fd) != openedServers.end())
                    {
                         connectClient(fd, clients, epollFd);
                         continue;
                    }
                    if (cgis.find(fd) != cgis.end())
                    {
                         CgiEvent(fd, epollFd, clients, cgis);
                         continue;
                    }
                    if (clients.find(fd) != clients.end() &&
                        (clients[fd]->getEvent().events & (EPOLLHUP | EPOLLRDHUP)))
                    {
                         std::cerr << "Client disconnected: fd=" << fd << std::endl;
                         epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL);
                         close(fd);
                         delete clients[fd];
                         clients.erase(fd);
                         continue;
                    }
                    handleClientRequest(clients, fd, servers, epollFd, cgis);
               }
               checkCgiTimeouts(epollFd, clients, cgis);
               timeout(clients, epollFd);
          }
          cleaningAfterSignal(clients, epollFd, openedServers, cgis);
     }
     catch (std::exception &e)
     {
          std::cerr << "Config error: " << e.what() << std::endl;
     }
}
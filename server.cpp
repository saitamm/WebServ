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
     typedef pair<string, int> AddrPort;
     set<AddrPort> interfaces;
     for (size_t i = 0; i < servers->size(); i++)
     {
          ConfigFile cfg = servers->at(i);
          string host = cfg.getHost();
          int port = cfg.getPort();
          stringstream ss;
          ss << port;
          string portStr = ss.str();
          pair<string, int> hp = make_pair(host, port);

          if (interfaces.find(hp) != interfaces.end())
               continue;
          interfaces.insert(hp);
          struct addrinfo hints, *res;
          memset(&hints, 0, sizeof(hints));
          hints.ai_family = AF_INET;
          hints.ai_socktype = SOCK_STREAM;
          hints.ai_flags = AI_PASSIVE;
          int status = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res);
          if (status != 0)
          {
               std::cerr << "getaddrinfo failed: " << std::endl;
               continue;
          }
          int serverSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
          cout << "the socket is opennn!\n";
          if (serverSocket == -1)
          {
               perror("socket");
               freeaddrinfo(res);
               continue;
          }
          int opt = 1;
          setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
          setNonBlocking(serverSocket);
          if (bind(serverSocket, res->ai_addr, res->ai_addrlen) < 0)
          {
               perror("bind");
               close(serverSocket);
               freeaddrinfo(res);
               continue;
          }

          listen(serverSocket, SOMAXCONN);
          struct epoll_event event;
          memset(&event, 0, sizeof(event));
          event.data.fd = serverSocket;
          event.events = EPOLLIN; 
          epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &event);
          openedServers[serverSocket] = cfg; 
          freeaddrinfo(res);
     }
     return (0);
}void connectClient(int fd, map<int, Client *> &clients, int epollFd)

{
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
                         clients[fd]->setStatus(Finished);
                         continue;
                    }
                    handleClientRequest(clients, fd, servers, epollFd, cgis);
               }
               checkCgiTimeouts(epollFd, clients, cgis);
               map<int, Client *>::iterator it;
               for (it = clients.begin(); it != clients.end();)
               {
                    Client *client = it->second;
                    int fd = it->first;
                    if (client->getStatus() == Finished)
                    {
                         epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL);
                         close(fd);
                         delete clients[fd];
                         clients.erase(it++);
                    }
                    else
                    {
                         ++it;
                    }
               }
               timeout(clients, epollFd);
          }
          cleaningAfterSignal(clients, epollFd, openedServers, cgis);
     }
     catch (std::exception &e)
     {
          std::cerr << "Config error: " << e.what() << std::endl;
     }
}
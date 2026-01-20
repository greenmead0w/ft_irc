#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <string>
#include <poll.h>       // For struct pollfd
#include <sys/socket.h> //socket functions
#include <netinet/in.h> // For sockaddr_in
#include <fcntl.h>      // For fcntl()
#include <unistd.h>     // For close()
#include <vector>
#include <errno.h>
#include <arpa/inet.h> // For inet_ntoa


class Server {
private:
    int                         _port;
    std::string                 _password;
    int                         _serverFd;      //listening socket
    std::vector<struct pollfd>  _fds;           //efficient multi-socket activity monitor
    std::map<int, Client>       _clients;       //Map FDs to Client objects

    // Forbidden Orthodox Canonical forms for now
    Server();
    Server(const Server &copy);
    Server &operator=(const Server &copy);

    //connection manager functions
    void acceptNewConnection();     // Helper for accept()
    void handleClientData(int fd);  // Helper for recv()
    void removeClient(int fd);      // Helper for disconnect



public:
    Server(int port, std::string password);
    ~Server();

    void init(); // The Setup
    void run();  // The Loop
};

#endif
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

class Server {
private:
    int                         _port;
    std::string                 _password;
    int                         _serverFd;      //listening socket
    std::vector<struct pollfd>  _fds;           //efficient multi-socket activity monitor

    // Forbidden Orthodox Canonical forms for now
    Server();
    Server(const Server &copy);
    Server &operator=(const Server &copy);

public:
    Server(int port, std::string password);
    ~Server();

    void init(); // The Setup
    void run();  // The Loop
};

#endif
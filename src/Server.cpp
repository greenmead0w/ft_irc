#include "Server.hpp"

Server::Server(int port, std::string password) : _port(port), _password(password), _serverFd(-1) {}

Server::~Server() {
    if (_serverFd != -1)
        close(_serverFd);
}

/* Initializes the IRC server's listening socket: creates an IPv4 TCP socket, 
enables port reuse and non-blocking I/O, binds it to all network interfaces 
on the specified port, and begins listening for client connections */
void Server::init() {
    //Create the listening socket (IPv4, TCP)
    _serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverFd < 0)
        throw std::runtime_error("Failed to create socket");

    //Allow immediate port reuse instead of having to wait ~2 mins when restarting server
    int yes = 1;
    if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
        throw std::runtime_error("Failed to set socket options");

    //Sets server socket funcs to Non-Blocking mode so they
    // return immediately instead of pausing the whole program
    if (fcntl(_serverFd, F_SETFL, O_NONBLOCK) < 0)
        throw std::runtime_error("Failed to set non-blocking");

    //Bind the socket to the port
    struct sockaddr_in address;
    address.sin_family = AF_INET;         //IPv4
    address.sin_addr.s_addr = INADDR_ANY; //Accept connections in all network interfaces
    
    //htons() converts the port number to "Network Byte Order"
    //(Big-Endian), which is how routers communicate
    address.sin_port = htons(_port);

    if (bind(_serverFd, (struct sockaddr *)&address, sizeof(address)) < 0)
        throw std::runtime_error("Failed to bind to port");

    //Start Listening
    //10 is the "backlog" (how many people can wait in line at once)
    if (listen(_serverFd, 10) < 0)
        throw std::runtime_error("Failed to listen");

    //add our Listening FD to the vector so poll() watches it
    struct pollfd serverPollFd;
    serverPollFd.fd = _serverFd;
    serverPollFd.events = POLLIN; // Watch for "Incoming data/connection"
    serverPollFd.revents = 0;
    _fds.push_back(serverPollFd);
}

// Minimal run() just to keep the program alive for testing
void Server::run() {
    extern bool g_stop; // From main.cpp
    std::cout << "Server is listening on port " << _port << "..." << std::endl;

    while (g_stop == false) {
        if (poll(&_fds[0], _fds.size(), -1) < 0 && g_stop == false)
            throw std::runtime_error("Poll failed");
    }
}
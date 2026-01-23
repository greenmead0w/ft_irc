#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <poll.h>       // for struct pollfd
#include <sys/socket.h> // socket functions
#include <netinet/in.h> // for sockaddr_in
#include <fcntl.h>      // for fcntl()
#include <unistd.h>     // for close()
#include <vector>
#include <errno.h>
#include <arpa/inet.h> // for inet_ntoa()
#include "Client.hpp"
#include "Command.hpp"

class Server {
private:
    int                         _port;
    std::string                 _password;
    int                         _serverFd;      //listening socket
    std::vector<struct pollfd>  _fds;           //efficient multi-socket activity monitor
    std::map<int, Client*>       _clients;       //table of client objects

    void acceptNewConnection(); 
    void handleClientData(int fd);
    void removeClient(int fd);
    void processCommand(int fd, std::string cmdLine);

    // Forbidden Orthodox Canonical forms for now
    Server();
    Server(const Server &copy);
    Server &operator=(const Server &copy);

public:
    Server(int port, std::string password);
    ~Server();

    void init(); //the setup
    void run();  //the loop
    void sendResponse(int fd);

    // Helpers
    void sendReply(int fd, const std::string& msg);
    std::string formatPrivmsg(Client* sender, const std::string& target, const std::string& text);

	// Getters
	std::string	getPassword() const;
	Client*		getClientByNickname(const std::string& nickname);
};

#endif
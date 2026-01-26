#include "Server.hpp"

Server::Server(int port, std::string password) : _port(port), _password(password), _serverFd(-1) {}

Server::~Server() {

    //delete every client from memory
    std::map<int, Client*>::iterator itC;
    for (itC = _clients.begin(); itC != _clients.end(); ++itC) 
        delete itC->second;
    _clients.clear();

	//Clear all channels on destructor
	std::map<std::string, Channel*>::iterator itCh;
	for (itCh = _channels.begin(); itCh != _channels.end(); ++itCh)
		delete itCh->second;
	_channels.clear();
	
	//Close main socket
	if (_serverFd != -1)
		close(_serverFd);
}

// Getters:
std::string	Server::getPassword() const { return _password; }

Client*		Server::getClientByNickname(const std::string& nickname) {
	std::map<int, Client*>::iterator it;
	for (it = _clients.begin(); it != _clients.end(); ++it) {
		if (it->second->getNickname() == nickname)
			return it->second;
	}
	return NULL;
}

//HELPERS

/* puts the message the server will send to the client
   in the _outgoingBuffer variable of the client*/
void Server::sendReply(int fd, const std::string& msg) {
    Client* client = _clients[fd];
    if (client) {
        client->appendOutgoingBuffer(msg);
    }
}

//priv message format is: :SenderNick!User@IP PRIVMSG Target :Message
std::string Server::formatPrivmsg(Client* sender, const std::string& target, const std::string& text) {
    
    return ":" + sender->getNickname() + "!" + sender->getUsername() + "@" + sender->getIP() 
           + " PRIVMSG " + target + " :" + text + "\r\n";
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

/* called when the listening socket FD triggers a POLLIN event (meaning a new client wants to connect)
   creates a new client socket, sets it to non-blocking and adds it to the fd-clients table */
void Server::acceptNewConnection() {
    struct sockaddr_in clientAddr; //stores where the connection is coming from (IP + port)
    socklen_t clientAddrLen = sizeof(clientAddr);

    //acepts connection
    int clientFd = accept(_serverFd, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientFd < 0) {
        std::cerr << "Error: accept() failed" << std::endl;
        return;
    }

    //set the new socket to non-blocking
    if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0) {
        std::cerr << "Error: fcntl() failed on new client" << std::endl;
        close(clientFd);
        return;
    }

    //add the new client socket FD to array of pollfds
    struct pollfd clientPollFd;
    clientPollFd.fd = clientFd;
    clientPollFd.events = POLLIN; //POLLIN = notify me whenever this FD becomes readable
    clientPollFd.revents = 0;
    _fds.push_back(clientPollFd);

    //create the client object, get their IP and add to the client table
    Client* newClient = new Client(clientFd);
    newClient->setIP(inet_ntoa(clientAddr.sin_addr));
    _clients[clientFd] = newClient;

    std::cout << "[Server] New connection from " << newClient->getIP() << " on FD " << clientFd << std::endl;

    //TESTING SERVER TO CLIENT COMMUNICATION
    _clients[clientFd]->appendOutgoingBuffer("Welcome to the IRC Server!\r\n");
    _clients[clientFd]->appendOutgoingBuffer("Please enter the PASS to continue.\r\n");
}

/* removes client from 1) channels 2)array of pollfds
   3) map of client FDs and closes the FD */
void Server::removeClient(int fd) {
    std::cout << "[Server] Client on FD " << fd << " disconnected." << std::endl;

    //remove client from every channel
    std::map<std::string, Channel*>::iterator it;
    for (it = _channels.begin(); it != _channels.end(); ++it) {
        it->second->removeClient(fd);
    }

    //remove from poll() array
    for (size_t i = 0; i < _fds.size(); i++) {
        if (_fds[i].fd == fd) {
            _fds.erase(_fds.begin() + i);
            break;
        }
    }

    //free memory + remove client from map
     if (_clients.count(fd)) {
        delete _clients[fd];
        _clients.erase(fd);
    }

    //close the FD
    close(fd);
}

// Pass the cmd to the command parser logic*/
void Server::processCommand(int fd, std::string cmdLine) {
	//Debugging:
    std::cout << "[DEBUG] Received from FD " << fd << ": [" << cmdLine << "]" << std::endl;
    Client* client = _clients[fd];
	if (!client)
		return ;
	
	Command cmd;
	cmd.parse(cmdLine);

	cmd.execute(client, this);
}


/* called when an existing client socket FD triggers a POLLIN event (meaning they sent a message)
   interprets client sent data either by removing if client disconnects or processing a cmd */
void Server::handleClientData(int fd) {
    char buffer[1024];

    //clear buffer with 0s (not required)
    for (int i = 0; i < 1024; i++) buffer[i] = 0;

    //read bytes from client and put them in buffer
    int bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);

    if (bytesRead <= 0) {
        //0 = client disconnects; < 0 error.
        removeClient(fd);
    } else {
        //add data to the client's internal buffer
        _clients[fd]->appendIncomingBuffer(buffer);

        std::string cmd;
        //loop keeps running while there's a full cmd to process
        while (!(cmd = _clients[fd]->getNextCommand()).empty()) {
            processCommand(fd, cmd);
        }
    }
}

/* sends data to appropriate client, removes sent data from buffer
   or finalizes if send errors not for full buffer reasons*/
void Server::sendResponse(int fd) {

    std::string &buffer = _clients[fd]->getOutgoingBuffer();
    if (buffer.empty()) return;

    // We try to send the whole buffer
    int bytesSent = send(fd, buffer.c_str(), buffer.size(), 0);

    if (bytesSent > 0) {
        //remove sent data from the buffer
        _clients[fd]->clearOutgoingBuffer(bytesSent);
    } else if (bytesSent == -1) {
        //EAGAIN, EWOULDBLOCK signal send not possible 
        //in that case we just wait for the next loop iteration
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            std::cerr << "Error: send() failed on FD " << fd << std::endl;
            removeClient(fd);
        }
    }
}

// Channel management:

// Search if a channel exists in the server using its name
Channel*	Server::getChannelByName(const std::string& name) {
	std::map<std::string, Channel*>::iterator it = _channels.find(name);
	if (it != _channels.end())
		return	it->second;
	return NULL;
}

void		Server::addChannel(const std::string&	name, Channel* chan) {
	//Check if exists to avoid duplicates
	if(_channels.find(name) == _channels.end())
		_channels[name] = chan;
}

void Server::run() {
    extern bool g_stop; // From main.cpp
    std::cout << "Server is listening on port " << _port << "..." << std::endl;

    while (g_stop == false) {

        //for every client, tell poll() to check for available sockets to write 
        // only if theres data to send
        for (size_t i = 1; i < _fds.size(); i++) {
            if (!_clients[_fds[i].fd]->getOutgoingBuffer().empty())
                _fds[i].events |= POLLOUT; 
            else
                _fds[i].events &= ~POLLOUT; //turn off pollout, avoids hammering CPU (because sockets are almost always writable)
        }

        // -1 means wait indefinitely for a signal or data
        if (poll(&_fds[0], _fds.size(), -1) < 0 && g_stop == false)
            break;

        //logic to handle POLLIN and/or POLLOUT
        for (size_t i = 0; i < _fds.size(); i++) {
            if (_fds[i].revents & POLLIN) {
                if (_fds[i].fd == _serverFd)
                    acceptNewConnection(); //it's a new client
                else
                    handleClientData(_fds[i].fd); //existing client
            }

            //handle writting to client
            if (_fds[i].revents & POLLOUT) {
                sendResponse(_fds[i].fd);
            }
        }
    }
}
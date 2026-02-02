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
    //_clients[clientFd]->appendOutgoingBuffer(":ircserv NOTICE * :*** Welcome to the IRC Server!\r\n");
    //_clients[clientFd]->appendOutgoingBuffer(":ircserv NOTICE * :*** Please enter the PASS to continue.\r\n");
    //_clients[clientFd]->appendOutgoingBuffer(":ircserv NOTICE * :*** PASS needed to continue.\r\n");
}

/* removes client from 1) channels 2)array of pollfds
   3) map of client FDs and closes the FD */
void Server::removeClient(int fd, const std::string& reason) {
    //std::cout << "[Server] Client on FD " << fd << " disconnected." << std::endl;

    //guard clause to prevent double deleting
    if (_clients.count(fd) == 0)
        return;

    Client* client = _clients[fd];
    std::string nick = client->getNickname();

    //format QUIT message for others: ":Nick!User@IP QUIT :reason"
    std::string quitMsg = ":" + nick + "!" + client->getUsername() + "@" + client->getIP() + " QUIT :" + reason + "\r\n";

    //prevent sending duplicate quit messages if client is in more than 1 shared channel
    std::set<int> notified;
    notified.insert(fd);

    //notify clients sharing channels + remove client from every channel
    std::map<std::string, Channel*>::iterator it = _channels.begin();
    while (it != _channels.end()) {

        if (it->second->isClientInChannel(fd)) {
            it->second->broadcast(quitMsg, this, fd, &notified); //tell other clients from channel someone is quitting
            it->second->removeClient(fd); //remove quitting client
        }
        
        //remove channel if empty
        if (it->second->getUserCount() == 0) {
            //std::cout << "[Server] Deleting empty channel: " << it->first << std::endl;
            std::cout << "[Server] Channel " << it->first << " is now empty and has been deleted" << std::endl;
            delete it->second;
            _channels.erase(it++); //post-increment important to prevent a dead iterator
        } else
            ++it;
    }

    //remove from poll() array
    for (size_t i = 0; i < _fds.size(); i++) {
        if (_fds[i].fd == fd) {
            _fds.erase(_fds.begin() + i);
            break;
        }
    }

    //free memory + remove client from map
    std::cout << "[Server] " << nick << " (FD " << fd << ") has quit." << std::endl;
    delete client;
    _clients.erase(fd);
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
int Server::handleClientData(int fd) {
    char buffer[1024];

    //clear buffer with 0s (not required)
    for (int i = 0; i < 1024; i++)
        buffer[i] = 0;

    //read bytes from client and put them in buffer
    int bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);

    if (bytesRead <= 0) {
        //0 = client disconnects; < 0 error.
        removeClient(fd);
        return -1;
    } else {
        //add data to the client's internal buffer
        _clients[fd]->appendIncomingBuffer(buffer);

        std::string cmd;
        //loop keeps running while there's a full cmd to process
        while (!(cmd = _clients[fd]->getNextCommand()).empty()) {
            processCommand(fd, cmd);

            //if cmd is QUIT then there's no client and function needs to stop
            if (_clients.find(fd) == _clients.end())
                return -1;
        }
    }
    return 0;
}

/* sends data to appropriate client, removes sent data from buffer
   or finalizes if send errors not for full buffer reasons*/
int Server::sendResponse(int fd) {

    if (_clients.count(fd) == 0)
        return -1;

    std::string &buffer = _clients[fd]->getOutgoingBuffer();
    if (buffer.empty())
        return 0;

    // We try to send the whole buffer
    int bytesSent = send(fd, buffer.c_str(), buffer.size(), 0);

    if (bytesSent > 0) {
        //remove sent data from the buffer
        _clients[fd]->clearOutgoingBuffer(bytesSent);
        return 0;
    } else if (bytesSent == -1) {
        //EAGAIN, EWOULDBLOCK signal send not possible 
        //in that case we just wait for the next loop iteration
        if (errno == EWOULDBLOCK || errno == EAGAIN)
            return 0;

        //bad errors: connection broken
        std::cerr << "Error: send() failed on FD " << fd << std::endl;
        removeClient(fd);
        return -1;
    }
    return 0;
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

// called to delete the channel when it has no clients
void Server::removeChannel(const std::string& name) {

    std::map<std::string, Channel*>::iterator it = _channels.find(name);

    if (it != _channels.end()) {
        std::cout << "[Server] Channel " << name << " is now empty and has been deleted" << std::endl;
        delete it->second;
        _channels.erase(it);
    }
}



//
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
            
            //POLLIN = incoming (read)
            if (_fds[i].revents & POLLIN) {
                if (_fds[i].fd == _serverFd) {
                    acceptNewConnection();
                } else {
                    if (handleClientData(_fds[i].fd) == -1) {
                        i--; //decrement index because next client takes the place from the previous client in array
                        continue; 
                    }
                }
            }

            //POLLOUT = outgoing (write)--- B. Handle Outgoing (Write) ---
            /* important: checking 'i < _fds.size()' because the POLLIN block above
               might have deleted the last element of the array*/
            if (i < _fds.size() && (_fds[i].revents & POLLOUT)) {
                if (sendResponse(_fds[i].fd) == -1) {
                    i--;
                    continue;
                }
            }

            //handle sudden crashes: if socket closes unexpectedly without sending data
            if (i < _fds.size() && (_fds[i].revents & (POLLERR | POLLHUP))) {
                removeClient(_fds[i].fd);
                i--;
                continue;
            }

			//Secure QUIT to avoid race condition and always show the ERROR message on user after disconnecting:
			if (i < _fds.size() && _clients.count(_fds[i].fd)) {
				Client* c = _clients[_fds[i].fd];
				if (c->isPendingDisconnect() && c->getOutgoingBuffer().empty()) {
					removeClient(_fds[i].fd);
					i--;
					continue;
				}
			}
        }
    }
}
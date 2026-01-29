#include "Channel.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include <iostream>


Channel::Channel() :
	_name(""),
	_topic(""),
	_userLimit(0),
	_modeInviteOnly(false),
	_modeTopicRestricted(true),
	_hasPassword(false),
	_hasLimit(false) {}

Channel::Channel(std::string name) :
	_name(name),
	_topic(""),
	_userLimit(0),
	_modeInviteOnly(false),
	_modeTopicRestricted(true),
	_hasPassword(false),
	_hasLimit(false) {}

Channel::~Channel() {}

Channel::Channel(const Channel &copy) :
	_name(copy._name),
	_topic(copy._topic),
	_password(copy._password),
	_userLimit(copy._userLimit),
	_modeInviteOnly(copy._modeInviteOnly),
	_modeTopicRestricted(copy._modeTopicRestricted),
	_hasPassword(copy._hasPassword),
	_hasLimit(copy._hasLimit),
	_clients(copy._clients),
	_operators(copy._operators) {}

Channel &Channel::operator=(const Channel &copy) {
	if (this != &copy) {
		this->_name = copy._name;
		this->_topic = copy._topic;
		this->_password = copy._password;
		this->_userLimit = copy._userLimit;
		this->_modeInviteOnly = copy._modeInviteOnly;
		this->_modeTopicRestricted = copy._modeTopicRestricted;
		this->_hasPassword = copy._hasPassword;
		this->_hasLimit = copy._hasLimit;
		this->_clients = copy._clients;
		this->_operators = copy._operators;
	}
	return *this;
}

// Management

void	Channel::addClient(Client* client) {
	if (client)
		_clients[client->getFd()] = client;
}

// Delete client from channel and also from operators list
void	Channel::removeClient(int fd) {
	_clients.erase(fd);
	_operators.erase(fd);
}

bool	Channel::isClientInChannel(int fd) const {
	return _clients.find(fd) != _clients.end();
}

void	Channel::addOperator(int fd) {
	_operators.insert(fd);
}

void	Channel::removeOperator(int fd) {
	_operators.erase(fd);
}

bool	Channel::isOperator(int fd) const {
	return _operators.find(fd) != _operators.end();
}

// Getters
std::string	Channel::getName() const { return _name; }
std::string	Channel::getTopic() const { return _topic; }
std::string	Channel::getPassword() const { return _password; }
size_t		Channel::getLimit() const { return _userLimit; }
size_t		Channel::getUserCount() const { return _clients.size(); }

bool		Channel::isInviteOnlyChannel() const { return _modeInviteOnly; }
bool		Channel::isTopicRestricted() const { return _modeTopicRestricted; }
bool		Channel::hasPassword() const { return _hasPassword; }
bool		Channel::hasLimit() const { return _hasLimit; }

const	std::map<int, Client*>& Channel::getClients() const { return _clients; }

// Setters
void		Channel::setTopic(const std::string &topic) { _topic = topic; }
void		Channel::setPassword(const std::string &pwd) {
	_password = pwd;
	_hasPassword = !pwd.empty();
}
void		Channel::setLimit(size_t	limit) {
	_userLimit = limit;
	_hasLimit = (limit > 0);
}
void		Channel::setInviteOnly(bool state) { _modeInviteOnly = state; }
void		Channel::setTopicRestricted (bool state) { _modeTopicRestricted = state; }

// COM
/* added the notified variable so client doesn't receive duplicate QUIT messages if
   they share >1 channel with client that is quitting */
void Channel::broadcast(const std::string &message, Server* server, int excludeFd, std::set<int>* notified) {

    std::map<int, Client*>::iterator it;
    for (it = _clients.begin(); it != _clients.end(); it++) {
        int targetFd = it->first;
        
        //skip sender
        if (targetFd == excludeFd)
            continue;
            
        //checks if client is already in set
        if (notified && notified->count(targetFd))
            continue;

        server->sendReply(targetFd, message);

        //add client to notified set
        if (notified)
            notified->insert(targetFd);
    }
}


//MODE

void Channel::addInvite(int fd) { 
	_invitedFds.insert(fd); 
}

void Channel::removeInvite(int fd) { 
	_invitedFds.erase(fd); 
}

bool Channel::isInvited(int fd) const { 
	return _invitedFds.count(fd); 
}


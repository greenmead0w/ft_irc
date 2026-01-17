#include "include/Channel.hpp"
#include <iostream>


Channel::Channel() :
	_name(""),
	_userLimit(0),
	_modeInviteOnly(false),
	_modeTopicRestricted(true),
	_hasPassword(false),
	_hasLimit(false) {}

Channel::Channel(std::string name) :
	_name(name),
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

void	Channel::addClient(Client* client);
void	Channel::removeClient(int fd);
bool	Channel::isClientInChannel(int fd) const;

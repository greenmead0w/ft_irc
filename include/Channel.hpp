#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <map>
#include <set>
#include <sys/socket.h>
#include "Client.hpp"

class Client;

class Channel {
private:
	std::string	_name;
	std::string	_topic;
	std::string	_password;
	size_t		_userLimit;

	// Channel modes
	bool	_modeInviteOnly; //+i
	bool	_modeTopicRestricted; //+t
	bool	_hasPassword; //k
	bool	_hasLimit;  //+l

	// Users management
	std::map<int, Client*>	_clients;
	std::set<int>			_operators;

public:
	Channel();
	Channel(std::string name);
	Channel(const Channel &copy);
	Channel &operator=(const Channel &copy);
	~Channel();

	// Getters
	std::string	getName() const;
	std::string	getTopic() const;
	std::string	getPassword() const;
	size_t		getLimit() const;
	size_t		getUserCount() const;

	bool		isInviteOnly() const;
	bool		isTopicRestricted() const;
	bool		hasPassword() const;
	bool		hasLimit() const;

	// Setters
	void		setTopic(const std::string &topic);
	void		setPassword(const std::string &pwd);
	void		setLimit(size_t	limit);
	void		setInviteOnly(bool state);
	void		setTopicRestricted (bool state);

	// Manage Clients
	void	addClient(Client* client);
	void	removeClient(int fd);
	bool	isClientInChannel(int fd) const;

	// Manage Operators
	void	addOperator(int fd);
	void	removeOperator(int fd);
	bool	isOperator(int fd) const;

	// COM
	void	broadcast(const std::string &message, int excludeFd = -1);

};

#endif
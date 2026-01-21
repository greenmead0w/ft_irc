#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <set>
#include <vector>

class Client {
private:
	int			_fd;
	std::string	_username;
	std::string	_nickname;
	std::string	_realname;
	std::string	_hostname;

	// Auth states
	bool		_hasEnteredPassword;
	bool		_isFullyRegistered;

	// Network management
	std::string	_incomingBuffer;
	std::string	_clientIP;

	// Permission & security
	bool	_isGlobalOperator;
	std::set<std::string>	_invitedTo;

public:
	Client();
	Client(int fd);
	Client(const Client &copy);
	Client &operator=(const Client &copy);
	~Client();

	// Getters
	int 		getFd() const;
	std::string getUsername() const;
	std::string getNickname() const;
	std::string getRealname() const;
	std::string getHostname() const;
	std::string getIP() const;
	std::string getBuffer() const;

	bool	hasEnteredPassword() const;
	bool	isFullyRegistered() const;
	bool	isGlobalOperator() const;

	// Setters
	void	setUsername(const std::string &user);
	void	setNickname(const std::string &nick);
	void	setRealname(const std::string &real);
	void	setHostname(const std::string &host);
	void	setIP(const std::string &ip);

	void	setEnteredPassword(bool state);
	void	setFullyRegistered(bool state);
	void	setGlobalOperator(bool state);

	/* Logic methods */
	// Buffer
	void	appendToBuffer(const std::string &data);
	void	clearBuffer();
	std::string getNextCommand();
	
	// Invitations
	void	addInvite(const std::string &channelName);
	void	removeInvite(const std::string &channelName);
	bool	isInvitedTo(const std::string &channelName) const;
};

#endif
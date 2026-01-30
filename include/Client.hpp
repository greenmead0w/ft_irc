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
	std::string	_incomingBuffer; //relative to server, what client sends
	std::string _outgoingBuffer; //data waiting to be sent to client
	std::string	_clientIP;

	// Permission & security
	bool	_isGlobalOperator;
	std::set<std::string>	_invitedTo;

	//Disconnection:
	bool		_pendingDisconnect;

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
	std::string getIncomingBuffer() const;
	std::string &getOutgoingBuffer();

	bool	hasEnteredPassword() const;
	bool	isFullyRegistered() const;
	bool	isGlobalOperator() const;
	bool	isPendingDisconnect() const;

	// Setters
	void	setUsername(const std::string &user);
	void	setNickname(const std::string &nick);
	void	setRealname(const std::string &real);
	void	setHostname(const std::string &host);
	void	setIP(const std::string &ip);

	void	setEnteredPassword(bool state);
	void	setFullyRegistered(bool state);
	void	setGlobalOperator(bool state);
	void	setPendingDisconnect(bool state);

	/* Logic methods */
	// incomingBuffer
	void	appendIncomingBuffer(const std::string &data);
	void	clearIncomingBuffer();
	std::string getNextCommand();

	//outgoingBuffer
	void        appendOutgoingBuffer(const std::string &msg);
    void        clearOutgoingBuffer(size_t sentBytes);
	
	// Invitations
	void	addInvite(const std::string &channelName);
	void	removeInvite(const std::string &channelName);
	bool	isInvitedTo(const std::string &channelName) const;
};

#endif
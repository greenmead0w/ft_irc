#include "Client.hpp"

Client::Client() : 
	_fd(-1),
	_hasEnteredPassword(false),
	_isFullyRegistered(false),
	_isGlobalOperator(false) {}

Client::Client(int fd) : 
	_fd(fd),
	_hasEnteredPassword(false),
	_isFullyRegistered(false),
	_isGlobalOperator(false) {}

Client::~Client() {}

// Canonical copy constructor:
Client::Client(const Client &copy) :
	_fd(copy._fd),
    _username(copy._username),
    _nickname(copy._nickname),
    _realname(copy._realname),
    _hostname(copy._hostname),
    _hasEnteredPassword(copy._hasEnteredPassword),
    _isFullyRegistered(copy._isFullyRegistered),
    _incomingBuffer(copy._incomingBuffer),
    _clientIP(copy._clientIP),
    _isGlobalOperator(copy._isGlobalOperator),
    _invitedTo(copy._invitedTo) {}

// Canonical assignation operator:
Client	&Client::operator=(const Client &copy) {
	if (this != &copy) {
		this->_fd = copy._fd;
		this->_username = copy._username;
		this->_nickname = copy._nickname;
		this->_realname = copy._realname;
		this->_hostname = copy._hostname;
		this->_hasEnteredPassword = copy._hasEnteredPassword;
		this->_isFullyRegistered = copy._isFullyRegistered;
		this->_incomingBuffer = copy._incomingBuffer;
		this->_clientIP = copy._clientIP;
		this->_isGlobalOperator = copy._isGlobalOperator;
		this->_invitedTo = copy._invitedTo;
	}
	return *this;
}

// Getters
int 		Client::getFd() const { return _fd; }
std::string Client::getUsername() const { return _username; }
std::string Client::getNickname() const { return _nickname; }
std::string Client::getRealname() const { return _realname; }
std::string Client::getHostname() const { return _hostname; }
std::string Client::getIP() const { return _clientIP; }
std::string Client::getIncomingBuffer() const { return _incomingBuffer; }
std::string &Client::getOutgoingBuffer() { return _outgoingBuffer; }

bool	Client::hasEnteredPassword() const { return _hasEnteredPassword; }
bool	Client::isFullyRegistered() const { return _isFullyRegistered; }
bool	Client::isGlobalOperator() const { return _isGlobalOperator; }

// Setters
void	Client::setUsername(const std::string &user) { _username = user; }
void	Client::setNickname(const std::string &nick) { _nickname = nick; }
void	Client::setRealname(const std::string &real) { _realname = real; }
void	Client::setHostname(const std::string &host){ _hostname = host; }
void	Client::setIP(const std::string &ip){ _clientIP = ip; }

void	Client::setEnteredPassword(bool state){ _hasEnteredPassword = state; }
void	Client::setFullyRegistered(bool state){ _isFullyRegistered = state; }
void	Client::setGlobalOperator(bool state){ _isGlobalOperator = state; }

// Logic methods

//Reconstruct partial commands, when CTRL+D
void	Client::appendIncomingBuffer(const std::string &data) {
	_incomingBuffer += data;
}

void	Client::clearIncomingBuffer() {
	_incomingBuffer.clear();
}

/* called by server to see if full message / cmd has been sent
   returns "" if buffer is empty or holds partial cmd
   cleans buffer up to \n */
std::string Client::getNextCommand() {
    size_t pos = _incomingBuffer.find("\n");
    if (pos == std::string::npos)
        return "";

    std::string cmd = _incomingBuffer.substr(0, pos);
    _incomingBuffer.erase(0, pos + 1);
    
    //Cleans \r if it exists (for \r\n endings)
    if (!cmd.empty() && cmd[cmd.size() - 1] == '\r')
        cmd.erase(cmd.size() - 1);
        
    return cmd;
}

void Client::appendOutgoingBuffer(const std::string &msg) {
    _outgoingBuffer += msg;
}

/* We might only send part of the buffer,
   so we remove only what was sent*/
void Client::clearOutgoingBuffer(size_t sentBytes) {
    _outgoingBuffer.erase(0, sentBytes);
}

void	Client::addInvite(const std::string &channelName) {
	_invitedTo.insert(channelName);
}
void	Client::removeInvite(const std::string &channelName) {
	_invitedTo.erase(channelName);
}
bool	Client::isInvitedTo(const std::string &channelName) const {
	return (_invitedTo.find(channelName) != _invitedTo.end());
}
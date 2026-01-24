#include "Command.hpp"
#include "Server.hpp"
#include "Channel.hpp"

void	Command::executeJOIN(Client* client, Server* server) {
	// Only registered users can Join a channel:
	if (!client->isFullyRegistered()) {
		// ERR_NOTREGISTERED - 451
		server->sendReply(client->getFd(), ":ircserv 451 * : You aren't registered yet\r\n");
		return ;
	}

	if (this->_params.empty()) {
		// ERR_NEEDMOREPARAMS - 461
		server->sendReply(client->getFd(), ":ircserv 461 " + client->getNickname() + " JOIN :Not enough parameters\r\n");
		return ;
	}

	std::string	chanName = _params[0];
	if (chanName[0] != '#') {
		// Channel name needs to start by # - IRC starndar
		server->sendReply(client->getFd(), ":Channel name needs to start by #\r\n");
		return ;
	}

	// Search if channel already exists on the server:
	Channel channel = server->
	//TODO Create channel map on server
}
#include "Command.hpp"
#include "Server.hpp"

void	Command::executePASS(Client* client, Server* server) {
	// ignore if the user is already registered
	if (client->isFullyRegistered())
		return ;

	// Check if any param have been sent
	if (this->_params.empty()) {
		// error 461: ERR_NEEDMOREPARAMS
		std::string	msg = ":ircserv 461 " + client->getNickname() + " PASS :Not Enough Parameters\r\n";
		//send(client->getFd(), msg.c_str(), msg.length(), 0);
		server->sendReply(client->getFd(), msg);
		return ;
	}

	if (this->_params[0] == server->getPassword()) {
		client->setEnteredPassword(true);
		server->sendReply(client->getFd(), ":ircserv NOTICE * :*** Password accepted. Please set your NICK or USER.\r\n");
	} else {
		// error 464: ERR_PASSWDMISMATCH
		std::string	msg = ":ircserv 464 " + client->getNickname() + " :Password incorrect\r\n";
		//send(client->getFd(), msg.c_str(), msg.length(), 0);
		server->sendReply(client->getFd(), msg);
		client->setEnteredPassword(false);
		server->sendReply(client->getFd(), "ERROR :Closing Link: Incorrect password\r\n");
		client->setPendingDisconnect(true);
	}
}

#include "Command.hpp"
#include "Server.hpp"

void	Command::executeUSER(Client* client, Server* server) {
	// We don't use server param, but let it here in case of future implementation.
	(void)server;

	if (!client->hasEnteredPassword()) {
		// ERR_NOTREGISTERED	
		std::string	msg = ":ircserv 451 * :User not registered\r\n";
        //send(client->getFd(), msg.c_str(), msg.length(), 0);
		server->sendReply(client->getFd(), msg);
        return;
	}

	//Avoid double register
	if (client->isFullyRegistered()) {
		//ERR_ALREADYREGISTERED
		std::string	msg = ":ircserv 462 :Already registered\r\n";
        //send(client->getFd(), msg.c_str(), msg.length(), 0);
		server->sendReply(client->getFd(), msg);
        return;
	}

	if (this->_params.size() < 4) {
		// ERR_NEEDMOREPARAMS	
		std::string	msg = ":ircserv 461 :Not enough parameters\r\n";
        //send(client->getFd(), msg.c_str(), msg.length(), 0);
		server->sendReply(client->getFd(), msg);
        return;
	}

	client->setUsername(_params[0]);
	client->setRealname(_params[3]);

	if (!client->getNickname().empty()) {
		if (!client->isFullyRegistered()) {
		client->setFullyRegistered(true);
		server->sendReply(client->getFd(), ":ircserv 001 " + client->getNickname() + " :Welcome to our IRC server :D\r\n");
		}
	} else {
		// Informative feedback message:
		server->sendReply(client->getFd(), ":ircserv NOTICE * :*** Username set. Now please send NICK <nickname> to complete registration.\r\n");
	}
}
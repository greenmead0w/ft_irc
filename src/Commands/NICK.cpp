#include "Command.hpp"
#include "Server.hpp"

void	Command::executeNICK(Client* client, Server* server) {
	
	std::string	currentNick = client->getNickname().empty() ? "*" : client->getNickname();

	if (!client->hasEnteredPassword()) {
		// If not, error 451 - ERR_NOTREGISTERED
		server->sendReply(client->getFd(), ":ircserv 451 " + currentNick + " :User not registered\r\n");
		return ;
	}
	// Check if nick param has been sent:
	if (this->_params.empty()) {
		// If not, error 431 - ERR_NONICKNAMEGIVEN
		server->sendReply(client->getFd(), ":ircserv 431 " + currentNick + "  :No nickname given\r\n");
		return ;
	}

	std::string newNick = _params[0];

	// Check if nickname is already used
	Client *existing = server->getClientByNickname(newNick);
	if (existing != NULL && existing->getFd() != client->getFd()) {
		// If already exists, error 433 - ERR_NICKNAMEINUSE	
		server->sendReply(client->getFd(), ":ircserv 433 " + currentNick + " " + newNick + " :Nickname already in use\r\n");
		return;
	}

	//Update nick:
	std::string oldNick = client->getNickname();
	client->setNickname(newNick);

	// Notify nickname change:
	std::string notify = ":" + (oldNick.empty() ? newNick : oldNick) + " NICK " + newNick + "\r\n";
	server->sendReply(client->getFd(), notify);

	// I decided to add this descriptive message ussing NOTICE to not break chat clients:
	if (!client->isFullyRegistered() && client->getUsername().empty())
		server->sendReply(client->getFd(), ":ircserv NOTICE * :*** Nickname set to " + newNick + ". Now please send USER <username> <mode> <unused> :<realname>\r\n");

	if (!client->getUsername().empty() && !client->isFullyRegistered()) {
		client->setFullyRegistered(true);
		//Welcome message RPL_WELCOME
		std::string	msg = ":ircserv 001 " + currentNick + "  :Welcome to our IRC server :D\r\n";
		//send(client->getFd(), msg.c_str(), msg.length(), 0);
		server->sendReply(client->getFd(), msg);
	}
}
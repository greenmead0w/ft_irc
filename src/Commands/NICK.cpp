#include "Command.hpp"
#include "Server.hpp"

void	Command::executeNICK(Client* client, Server* server) {
	
	std::string	currentNick = client->getNickname().empty() ? "*" : client->getNickname();

	if (!client->hasEnteredPassword()) {
		// If not, error 451 - ERR_NOTREGISTERED
		std::string	msg = ":ircserv 451 " + currentNick + " :User not registered\r\n";
		send(client->getFd(), msg.c_str(), msg.length(), 0);
		return ;
	}
	// Check if nick param has been sent:
	if (this->_params.empty()) {
		// If not, error 431 - ERR_NONICKNAMEGIVEN
		std::string	msg = ":ircserv 431 " + currentNick + "  :No nickname given\r\n";
		send(client->getFd(), msg.c_str(), msg.length(), 0);
	}

	std::string newNick = _params[0];

	// Check if nickname is already used
	Client *existing = server->getClientByNickname(newNick);
	if (existing != NULL && existing->getFd() != client->getFd()) {
		// If already exists, error 433 - ERR_NICKNAMEINUSE	
		std::string	msg = ":ircserv 433 " + currentNick + "  :Nickname alredy in use\r\n";
		send(client->getFd(), msg.c_str(), msg.length(), 0);
		return;
	}

	//Update nick:
	std::string oldNick = client->getNickname();
	client->setNickname(newNick);

	// Notify nickname change:
	std::string notify = ":" + (oldNick.empty() ? newNick : oldNick) + " NICK " + newNick + "\r\n";
	send(client->getFd(), notify.c_str(), notify.length(), 0);

	if (!client->getUsername().empty() && !client->isFullyRegistered()) {
		client->setFullyRegistered(true);
		//Welcome message RPL_WELCOME
		std::string	msg = ":ircserv 001 " + currentNick + "  :Welcome to our IRC server :D\r\n";
		send(client->getFd(), msg.c_str(), msg.length(), 0);
	}
}
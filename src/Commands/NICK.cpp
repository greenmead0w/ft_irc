#include "Command.hpp"
#include "Server.hpp"

void	Command::executeNICK(Client* client, Server* server) {
	if (!client->hasEnteredPassword()) {
		// If not, error 451 - ERR_NOTREGISTERED
		std::string	msg = ":ircserv 451 " + client->getNickname() + " :User not registered\r\n";
		send(client->getFd(), msg.c_str(), msg.length(), 0);
		return ;
	}
	// Check if nick param has been sent:
	if (this->_params.empty()) {
		// If not, error 431 - ERR_NONICKNAMEGIVEN
		std::string	msg = ":ircserv 431 " + client->getNickname() + "  :No nickname given\r\n";
		send(client->getFd(), msg.c_str(), msg.length(), 0);
	}

	std::string newNick = _params[0];

	// Check if nickname is already used
	// TODO: Create server->getClientByNickname(nick);
	// We should create a std::map of _clients to Server

	//Update nick:
	std::string oldNick = client->getNickname();

	// Notify nickname change:
	std::string notify = ":" + (oldNick.empty() ? newNick : oldNick) + " NICK " + newNick + "\r\n";
	send(client->getFd(), notify.c_str(), notify.length(), 0);

	if (!client->getUsername().empty() && !client->isFullyRegistered()) {
		client->setFullyRegistered(true);
		//TODO: welcome message 001
	}
}
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
	Channel *channel = server->getChannelByName(chanName);
	bool	isNewChannel = false;

	if (channel == NULL) {
		channel = new Channel(chanName);
		server->addChannel(chanName, channel);
		isNewChannel = true;
	}

	// Verify if user is already on channel or not:
	if (channel->isClientInChannel(client->getFd())) {
		server->sendReply(client->getFd(), ":Client is already in channel\r\n");
		return ;
	}

	// Add client to the channel: 
	channel->addClient(client);
	if (isNewChannel) {
		channel->addOperator(client->getFd());
	}

	// Lets send a broadcast to the channel notifying the JOIN:
	std::string joinBroadcast = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost JOIN " + chanName + "\r\n";
	channel->broadcast(joinBroadcast, server);

	// Send topic if exist:
	if (!channel->getTopic().empty()) {
		// 332 - RPL_TOPIC
		server->sendReply(client->getFd(), ":ircserv 332 " + client->getNickname() + " " + chanName + " :" + channel->getTopic() + "\r\n");
	}

	// Lets send a list of all current users inside the channel:
	// RPL_NAMREPLY
	std::string	names = ":ircserv 353 " + client->getNickname() + " = " + chanName + " :";
	const std::map<int, Client*>& users = channel->getClients();
	std::map<int, Client*>::const_iterator it;

	for (it = users.begin(); it != users.end(); it++) {
		if (it != users.begin())
			names += " ";
		if (channel->isOperator(it->first))
			names += "@";
		
		names += it->second->getNickname();
	}
	names += "\r\n";
	server->sendReply(client->getFd(), names);

	// End of names list - 366 -- RPL_ENDOFNAMES
	server->sendReply(client->getFd(), ":ircserv 366 " + client->getNickname() + " " + chanName + " :End of /NAMES list\r\n");
}
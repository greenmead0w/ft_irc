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
	std::string providedPass = (_params.size() > 1) ? _params[1] : ""; //check if password provided

	if (chanName[0] != '#') {
		// Channel name needs to start by # - IRC standar
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
	} else {

		// Verify if user is already on channel or not:
		if (channel->isClientInChannel(client->getFd())) {
		server->sendReply(client->getFd(), ":Client is already in channel\r\n");
		return ;
		}
		
		//check if can only join via invite
		if (channel->isInviteOnlyChannel() && !channel->isInvited(client->getFd())) {
            server->sendReply(client->getFd(), ":ircserv 473 " + client->getNickname() + " " + chanName + " :Cannot join channel (+i)\r\n");
            return;
        }

		//check if channel requires pwd to join (+k)
		if (channel->hasPassword() && providedPass != channel->getPassword()) {
            server->sendReply(client->getFd(), ":ircserv 475 " + client->getNickname() + " " + chanName + " :Cannot join channel (+k)\r\n");
            return;
        }

		//check if channel has limit number of users
		if (channel->hasLimit() && channel->getUserCount() >= channel->getLimit()) {
            server->sendReply(client->getFd(), ":ircserv 471 " + client->getNickname() + " " + chanName + " :Cannot join channel (+l)\r\n");
            return;
        }

	}

	// Add client to the channel: 
	channel->addClient(client);
	if (isNewChannel) {
		channel->addOperator(client->getFd());
	}

	//remove invite if client used one to join
	if (channel->isInvited(client->getFd()))
		channel->removeInvite(client->getFd());


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
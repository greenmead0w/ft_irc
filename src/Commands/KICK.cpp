#include "Command.hpp"
#include "Server.hpp"
#include "Channel.hpp"

void	Command::executeKICK(Client* client, Server* server) {
	if (_params.size() < 2) {
		server->sendReply(client->getFd(), ":ircserv 461 " + client->getNickname() + " KICK :Not enough parameters\r\n");
		server->sendReply(client->getFd(), ":ircserv NOTICE * :*** Usage: KICK <#channel> <nickname> [:reason]\r\n");
		return;
	}

	std::string	chanName = _params[0];
	std::string	targetNick = _params[1];

	// If there is a reason, it is the second param. If not, a default is used:
	std::string	kickReason = (_params.size() > 2) ? _params[2] : "No reason given";

	Channel	*chan = server->getChannelByName(chanName);

	// If channel not exist: ERR_NOSUCHCHANNEL
	if(!chan) {
		server->sendReply(client->getFd(), ":ircserv 403 " + client->getNickname() + " " + chanName + " :No such channel\r\n");
		return;
	}

	//Are you on the channel? if not: ERR_NOTONCHANNEL
	if (!chan->isClientInChannel(client->getFd())) {
		server->sendReply(client->getFd(), ":ircserv 442 " + client->getNickname() + " " + chanName + " :You're not on that channel\r\n");
        return;
	}

	//Are you an operator on this channel? If not: ERR_CHANOPRIVSNEEDED
	if (!chan->isOperator(client->getFd())) {
		server->sendReply(client->getFd(), ":ircserv 482 " + client->getNickname() + " " + chanName + " :You're not channel operator\r\n");
        server->sendReply(client->getFd(), ":ircserv NOTICE " + client->getNickname() + " :*** Only operators can use KICK.\r\n");
        return;
	}

	//Is the target on the channel? if not: ERR_USERNOTINCHANNEL
	Client *targetClient = server->getClientByNickname(targetNick);
	if (!targetClient || !chan->isClientInChannel(targetClient->getFd())){
		server->sendReply(client->getFd(), ":ircserv 441 " + client->getNickname() + " " + targetNick + " " + chanName + " :They aren't on that channel\r\n");
        return;
	}

	// Lets kick that user! >:D
	// Kick message format should be: :Kicker!User@Host KICK #Channel Target :Reason
	std::string kickMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost KICK " + chanName + " " + targetNick + " :" + kickReason + "\r\n";
	chan->broadcast(kickMsg, server);
	// Kick him: 
	chan->removeClient(targetClient->getFd());
	// Send a optional confirmation to the operator
	server->sendReply(client->getFd(), ":ircserv NOTICE " + client->getNickname() + " :*** You kicked " + targetNick + " from " + chanName + "\r\n");
}
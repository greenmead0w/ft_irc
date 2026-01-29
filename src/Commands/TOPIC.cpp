#include "Command.hpp"
#include "Server.hpp"
#include "Channel.hpp"

void	Command::executeTOPIC(Client* client, Server* server) {
	if (_params.empty()) {
		// ERR_NEEDMOREPARAMS
		server->sendReply(client->getFd(), ":ircserv 461 " + client->getNickname() + " TOPIC :Not enough parameters\r\n");
		// Feedback NOTICE:
		server->sendReply(client->getFd(), ":ircserv NOTICE * :*** Usage: TOPIC <#channel> [<new_topic>]\r\n");
		return;
	}

	std::string chanName = _params[0];
	Channel *chan = server->getChannelByName(chanName);
	// Channel exists?
	if (!chan) {
		//ERR_NOSUCHCHANNEL
		server->sendReply(client->getFd(), ":ircserv 403 " + client->getNickname() + " " + chanName + " :No such channel\r\n");
		return;
	}

	//Are you in the channel?
	if (!chan->isClientInChannel(client->getFd())) {
		//ERR_NOTONCHANNEL
		server->sendReply(client->getFd(), ":ircserv 442 " + client->getNickname() + " " + chanName + " :You're not on that channel\r\n");
		return;
	}

	// If only 1 parameter is sent, we send the TOPIC of the channel, but we don't change it:
	if (_params.size() == 1) {
		if (chan->getTopic().empty()) {
			//RPL_NOTOPIC
			server->sendReply(client->getFd(), ":ircserv 331 " + client->getNickname() + " " + chanName + " :No topic is set\r\n");
		} else {
			//RPL_TOPIC
			server->sendReply(client->getFd(), ":ircserv 332 " + client->getNickname() + " " + chanName + " :" + chan->getTopic() + "\r\n");
		}
		return;
	}

	// Change the topic:
	std::string	newTopic = _params[1];
	//Verify privileges if +t mode is enabled:
	if (chan->isTopicRestricted() && !chan->isOperator(client->getFd())) {
		//ERR_CHANOPRIVSNEEDED
		server->sendReply(client->getFd(), ":ircserv 482 " + client->getNickname() + " " + chanName + " :You're not channel operator\r\n");
        server->sendReply(client->getFd(), ":ircserv NOTICE " + client->getNickname() + " :*** This channel has topic protection (+t) active.\r\n");
        return;
	}

	// If possible, change Topic and inform all the channel:
	chan->setTopic(newTopic);
	std::string topicBroadcast = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost TOPIC " + chanName + " :" + newTopic + "\r\n";
	chan->broadcast(topicBroadcast, server);
}
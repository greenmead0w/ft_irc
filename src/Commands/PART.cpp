#include "Server.hpp"
#include "Command.hpp"

void Command::executePART(Client* client, Server* server) {

    if (_params.empty()) {
        //ERR_NEEDMOREPARAMS - 461
        server->sendReply(client->getFd(), ":ircserv 461 " + client->getNickname() + " PART :Not enough parameters\r\n");
        return;
    }

    std::string channelName = _params[0];

    Channel* chan = server->getChannelByName(channelName);

    if (!chan) {
        //ERR_NOSUCHCHANNEL - 403
        server->sendReply(client->getFd(), ":ircserv 403 " + client->getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }

    if (!chan->isClientInChannel(client->getFd())) {
        //ERR_NOTONCHANNEL - 442
        server->sendReply(client->getFd(), ":ircserv 442 " + client->getNickname() + " " + channelName + " :Client not on channel\r\n");
        return;
    }

    //notify everyone in the channel, including client
    //format: :Nick!User@Host PART #channel [:reason]
    std::string reason = (_params.size() > 1) ? _params[1] : "Leaving";
    std::string msg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getIP() + " PART " + channelName + " :" + reason + "\r\n";
    
    chan->broadcast(msg, server);

    //remove client from channel
    chan->removeClient(client->getFd());
    
    //check if channel is now empty
    if (chan->getUserCount() == 0) {
        server->removeChannel(channelName);
    }
}
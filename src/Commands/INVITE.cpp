
#include "Server.hpp"
#include "Command.hpp"


void Command::executeINVITE(Client* client, Server* server) {

    if (_params.size() < 2) { 
        // ERR_NEEDMOREPARAMS - 461
		server->sendReply(client->getFd(), ":ircserv 461 " + client->getNickname() + " INVITE :Not enough parameters\r\n");
        return; 
    }

    std::string targetNick = _params[0]; //client to invite
    std::string chanName = _params[1]; //channel to invite to
    Channel* chan = server->getChannelByName(chanName);

    if (!chan) { 
        // ERR_NOSUCHCHANNEL - 403
        server->sendReply(client->getFd(), ":ircserv 403 " + client->getNickname() + " " + chanName + " :No such channel\r\n");
        return; 
    }

    //only channel operators can invite other clients
    if (!chan->isOperator(client->getFd())) {
        // ERR_CHANOPRIVSNEEDED - 482
        server->sendReply(client->getFd(), ":ircserv 482 " + client->getNickname() + " " + chanName + " :You're not channel operator\r\n");
        return;
    }

    Client* targetClient = server->getClientByNickname(targetNick);
    if (!targetClient) { 
        // ERR_NOSUCHNICK - 401
        server->sendReply(client->getFd(), ":ircserv 401 " + client->getNickname() + " " + targetNick + " :No such nick\r\n");
        return; 
    }

    if (chan->isClientInChannel(targetClient->getFd())) {
        // ERR_USERONCHANNEL - 443
        server->sendReply(client->getFd(), ":ircserv 443 " + client->getNickname() + " " + targetNick + " " + chanName + " :is already on channel\r\n");
        return;
    }

    //add target to channel invite list
    chan->addInvite(targetClient->getFd());
    
    //notify the target client
    server->sendReply(targetClient->getFd(), ":" + client->getNickname() + " INVITE " + targetNick + " :" + chanName + "\r\n");

    //notify the sender 
    // RPL_INVITING - 341
    server->sendReply(client->getFd(), ":ircserv 341 " + client->getNickname() + " " + targetNick + " " + chanName + "\r\n");
}
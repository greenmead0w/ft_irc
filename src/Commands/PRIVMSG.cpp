#include "Command.hpp"
#include "Server.hpp"


/* first validate if sender is registered and message has required params,
   then solve for channel message or private message between clients */
void Command::executePRIVMSG(Client* client, Server* server) {

    if (!client->isFullyRegistered()) {
        //ERR_NOTREGISTERED - 451
        std::string	msg = ":ircserv 451 * :User not registered\r\n";
        server->sendReply(client->getFd(), msg);
        return;
    }

    //expected params: PRIVMSG <target> :<message>
    if (_params.empty()) {
        server->sendReply(client->getFd(),
            ":ircserv 411 " + client->getNickname() + " :No recipient given (PRIVMSG)\r\n");
        return;
    }
    if (_params.size() == 1) {
        server->sendReply(client->getFd(),
            ":ircserv 412 " + client->getNickname() + " :No text to send\r\n");
        return;
    }

    std::string target= _params[0];

    if (target[0] == '#'){
        Channel *chan = server->getChannelByName(target);
        
        if (!chan) {
            //ERR_NOSUCHCHANNEL - 403
            server->sendReply(client->getFd(), ":ircserv 403 " + client->getNickname() + " " + target + " :No such channel\r\n");
            return;
        }
        
        if (!chan->isClientInChannel(client->getFd())) {
            //ERR_CANNOTSENDTOCHAN - 404
            server->sendReply(client->getFd(), ":ircserv 404 " + client->getNickname() + " " + target + " :Cannot send to channel\r\n");
            return;
        }

        // Broadcast to everyone except the sender. localhost hardcoded for format
        std::string formatted = ":" + client->getNickname() + "!" + 
            client->getUsername() + "@localhost PRIVMSG " + target + " :" + _params[1] + "\r\n";
        chan->broadcast(formatted, server, client->getFd());

    }

    else { 
        //find the target client
        Client* targetClient = server->getClientByNickname(target);
        
        if (targetClient == NULL) {
            //ERR_NOSUCHNICK
            std::string	msg = ":ircserv 401 " + client->getNickname() + " " + target + " :No such nick\r\n";
            server->sendReply(client->getFd(), msg);
            return;
        }

        //format the message for the receiver using helper
        std:: string formatted = server->formatPrivmsg(client, _params[0], _params[1]);

        //send message to target
        server->sendReply(targetClient->getFd(), formatted);

    }

}
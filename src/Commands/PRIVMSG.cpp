#include "Command.hpp"
#include "Server.hpp"


/* 1) controls if sender is registered and message has required params
   2) finds the target client, 3) extracts the message, 4) formats the message
   5) send the message*/
void Command::executePRIVMSG(Client* client, Server* server) {

    if (!client->isFullyRegistered()) {
        //ERR_NOTREGISTERED
        std::string	msg = ":ircserv 451 * :User not registered\r\n";
        server->sendReply(client->getFd(), msg);
        return;
    }

    //expected params: PRIVMSG <target> :<message>
    if (_params.size() < 2) {
        //ERR_NORECIPIENT
        std::string	msg = ":ircserv 411 " + client->getNickname() + " :No recipient given (PRIVMSG)\r\n";
        server->sendReply(client->getFd(), msg);
        return;
    }

    std::string targetNick = _params[0];
    
    //find the target client
    Client* targetClient = server->getClientByNickname(targetNick);
    
    if (targetClient == NULL) {
        //ERR_NOSUCHNICK
        std::string	msg = ":ircserv 401 " + client->getNickname() + " " + targetNick + " :No such nick\r\n";
        server->sendReply(client->getFd(), msg);
        return;
    }

    //format the message for the receiver using helper
    std:: string formatted = server->formatPrivmsg(client, _params[0], _params[1]);

    //send message to target
    server->sendReply(targetClient->getFd(), formatted);
}
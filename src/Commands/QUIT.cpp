#include "Server.hpp"
#include "Command.hpp"


/* check if client sent reason for quitting, send a message to client to confirm quitting
   call the removeClient() cleaning function*/
void Command::executeQUIT(Client* client, Server* server) {
    
    std::string reason = _params.empty() ? "Client Quit" : _params[0];
    
    server->sendReply(client->getFd(), "ERROR :Closing Link: " + client->getNickname() + " (" + reason + ")\r\n");

    //server->removeClient(client->getFd(), reason); -> J: Moving it to main loop to avoid race condition
	client->setPendingDisconnect(true);
}

#include "Command.hpp"
#include "Server.hpp"
#include <sstream> //convert getLimit() to string
#include <cstdlib> //for atoi()


/* MODE parsing helper. Some flags require parameters (k, l, o) and others don't (i, t)
*/
bool Command::applyChannelModes(Server* server, Channel* chan, std::string& appliedModes, std::string& modeParams) 
{
    // MODE #room +l 10
    //_params[1] == flag(s)
    std::string modeString = _params[1];

    //default or '+' is 'true'; else is 'false' 
    bool adding = true;

    // indexes into flag arguments (not always present)
    // paramIdx = 0 is channel ; paramIdx = 1 is list of flags ; paramIdx = 2 are flag params
    size_t paramIdx = 2;

    //loop char by char the flag list
    for (size_t i = 0; i < modeString.length(); i++) {
        char c = modeString[i];

        if (c == '+') {
            adding = true;
            continue;
        }
        if (c == '-') {
            adding = false;
            continue;
        }

        if (appliedModes.empty() ||
            (adding && appliedModes[appliedModes.size() - 1] != '+') ||
            (!adding && appliedModes[appliedModes.size() - 1] != '-')) {
            appliedModes += (adding ? "+" : "-");
        }

        //invite flag
        if (c == 'i') {
            chan->setInviteOnly(adding);
            appliedModes += "i";
        }

        //topic flag
        else if (c == 't') {
            chan->setTopicRestricted(adding);
            appliedModes += "t";
        }

        //password flag
        else if (c == 'k') {
            //set pwd
            if (adding && paramIdx < _params.size()) { //_params.size >= 3 if flag params exist
                std::string key = _params[paramIdx++];
                chan->setPassword(key);
                appliedModes += "k";
                modeParams += " " + key;
            }
            //remove pwd from channel
            else if (!adding) {
                chan->setPassword("");
                appliedModes += "k";
            }
        }

        //user limit flag
        else if (c == 'l') {
            if (adding && paramIdx < _params.size()) {
                int limit = std::atoi(_params[paramIdx++].c_str());
                chan->setLimit(limit);
                appliedModes += "l";
                modeParams += " " + _params[paramIdx - 1];
            }
            //no user limit
            else if (!adding) {
                chan->setLimit(0);
                appliedModes += "l";
            }
        }

        // channel operator flag
        else if (c == 'o') {
            if (paramIdx < _params.size()) {
                Client* targetClient = server->getClientByNickname(_params[paramIdx++]);
                if (targetClient && chan->isClientInChannel(targetClient->getFd())) {
                    if (adding)
                        chan->addOperator(targetClient->getFd());
                    else
                        chan->removeOperator(targetClient->getFd());

                    appliedModes += "o";
                    modeParams += " " + targetClient->getNickname();
                }
            }
        }
    }

    return (appliedModes.size() > 1);
}




void Command::executeMODE(Client* client, Server* server) {
    // ERR_NEEDMOREPARAMS - 461
    if (_params.size() < 1) {
        server->sendReply(client->getFd(), ":ircserv 461 " + client->getNickname() + " MODE :Not enough parameters\r\n");
        return;
    }

    std::string target = _params[0];

    //only channel mode supported
    if (target[0] != '#')
        return;

    Channel* chan = server->getChannelByName(target);
    if (!chan) {
        // ERR_NOSUCHCHANNEL - 403
        server->sendReply(client->getFd(), ":ircserv 403 " + client->getNickname() + " " + target + " :No such channel\r\n");
        return;
    }

    // View MODE cmd request (doesn't return operator list)
    if (_params.size() == 1) {
        std::string modeList = "+";
        std::string modeParams;

        if (chan->isInviteOnlyChannel())
            modeList += "i";
        if (chan->isTopicRestricted())
            modeList += "t";
        if (chan->hasPassword())
            modeList += "k"; // not adding password here for security reasons
        if (chan->hasLimit()) {
            modeList += "l";

            //convert size_t to string
            std::ostringstream oss;
            oss << chan->getLimit();
            modeParams += " " + oss.str();
        }

        // 	RPL_CHANNELMODEIS - 324
        server->sendReply(client->getFd(), ":ircserv 324 " + client->getNickname() + " " + target + " " + modeList + modeParams + "\r\n");
        
        return;
    }

    // "Write" MODE request
    // only channel operators can use MODE to modify the Channel
    if (!chan->isOperator(client->getFd())) {
        // ERR_CHANOPRIVSNEEDED - 482
        server->sendReply(client->getFd(), ":ircserv 482 " + client->getNickname() + " " + target + " :You're not channel operator\r\n");
        return;
    }

    std::string appliedModes;
    std::string modeParams;

    // only broadcast if properties in Channel have been modified
    if (applyChannelModes(server, chan, appliedModes, modeParams)) {
        std::string notify = ":" + client->getNickname() + " MODE " + target + " " + appliedModes + modeParams + "\r\n";
        chan->broadcast(notify, server);
    }
}
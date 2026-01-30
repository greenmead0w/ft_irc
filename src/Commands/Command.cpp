#include "Command.hpp"
#include "Server.hpp"

Command::Command() : 
	_rawLine(""),
	_cmdName("") {initHandlers();}

Command::Command(const Command &copy) : 
	_rawLine(copy._rawLine),
	_cmdName(copy._cmdName) {initHandlers();}

Command &Command::operator=(const Command &copy) {
	if (this != &copy) {
		_rawLine = copy._rawLine;
		_cmdName = copy._cmdName;
	}
	return *this;
}
Command::~Command() {}

/* fill the function map on construction  */
void Command::initHandlers() {
    _handlers["PASS"] = &Command::executePASS;
    _handlers["NICK"] = &Command::executeNICK;
    _handlers["USER"] = &Command::executeUSER;
    _handlers["PRIVMSG"] = &Command::executePRIVMSG;
    _handlers["JOIN"] = &Command::executeJOIN;
    _handlers["PART"] = &Command::executePART;
    _handlers["QUIT"] = &Command::executeQUIT;
    _handlers["KICK"] = &Command::executeKICK;
    _handlers["TOPIC"] = &Command::executeTOPIC;
    _handlers["MODE"] = &Command::executeMODE;
    _handlers["INVITE"] = &Command::executeINVITE;
}

// Parses a raw command line into a command name and its params
void Command::parse(const std::string &line) {
	this->_rawLine = line;
	std::string trimmed = line;

	// first clean spaces and line breaks
	size_t	first = trimmed.find_first_not_of("\n\r\t");
	if (first == std::string::npos)
		return ;
	size_t	last = trimmed.find_last_not_of("\n\r\t");
	trimmed = trimmed.substr(first, (last - first + 1));

	// extact the command
	size_t	space_pos = trimmed.find(' ');
	if (space_pos == std::string::npos) {
		this->_cmdName = trimmed;
		return ;
	}
	this->_cmdName = trimmed.substr(0, space_pos);
	std::string remaining = trimmed.substr(space_pos + 1);

	// extract params
	while (!remaining.empty()) {
		size_t	start = remaining.find_first_not_of(' ');
		if (start == std::string::npos)
			break;
		remaining = remaining.substr(start);

		// if starts from ':' the remain is only 1 parameter
		if (remaining[0] == ':') {
			this->_params.push_back(remaining.substr(1));
			break;
		}
		// if there are not more spaces, remaining is the last parameter.
		size_t next_space = remaining.find(' ');
		if (next_space == std::string::npos) {
			this->_params.push_back(remaining);
			break;
		} else {
			this->_params.push_back(remaining.substr(0, next_space));
			remaining = remaining.substr(next_space + 1);
		}
	}
}


void Command::execute(Client* client, Server* server) {
    
	//cmd to MAYUS
    for (size_t i = 0; i < _cmdName.length(); ++i)
        _cmdName[i] = toupper(_cmdName[i]);

    //check if cmd found in map
    if (_handlers.count(_cmdName)) {
        cmdHandler handler = _handlers[_cmdName];

        //we only accept cmds if client has already entered PASS
        if (_cmdName != "PASS" && !client->hasEnteredPassword()) {
            server->sendReply(client->getFd(), ":ircserv 451 * :You have not registered\r\n");
            return;
        }

        //execute function by calling pointer
        (this->*handler)(client, server);
    } 
    else {
        //si ponen comando que no existe, igual hay que mejorar el mensaje
		//ERR_UNKNOWNCOMMAND - 421
		server->sendReply(client->getFd(), ":ircserv 421 * :Unknown command used\r\n");
    }
}

// void Command::execute(Client* client, Server* server) {
// 	if (_cmdName == "PASS") 
// 		this->executePASS(client, server);
// 	// Only if pass return exit:
// 	else if (client->hasEnteredPassword()) {
// 		if (_cmdName == "NICK")
// 			this->executeNICK(client, server);
// 		else if (_cmdName == "USER")
// 			this->executeUSER(client, server);
// 		else if (_cmdName == "PRIVMSG")
//             this->executePRIVMSG(client, server);
// 		else if (_cmdName == "JOIN")
//             this->executeJOIN(client, server);	
// 		else if (_cmdName == "PART")
//             this->executePART(client, server);		
// 		else if (_cmdName == "QUIT")
// 			this->executeQUIT(client, server);
// 		else if (_cmdName == "KICK")
// 			this->executeKICK(client, server);
// 		else if (_cmdName == "TOPIC")
// 			this->executeTOPIC(client, server);
// 		else if (_cmdName == "MODE")
// 			this->executeMODE(client, server);
// 		else if (_cmdName == "INVITE")
// 			this->executeINVITE(client, server);
// 	}
		
// }

std::string	Command::getCmdName() const { return _cmdName; }
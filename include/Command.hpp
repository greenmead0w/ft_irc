#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "Client.hpp"
#include "Channel.hpp"
#include <string>
#include <vector>


class Server;

class Command {
private:
	std::string					_rawLine;
	std::string					_cmdName;
	std::vector<std::string>	_params;

	
	typedef void (Command::*cmdHandler)(Client*, Server*); //?

	std::map<std::string, cmdHandler> _handlers; //map of cmds to functions

	void initHandlers(); //fills functions in cmd map

	//executeMODE parsing helper
	bool	applyChannelModes(Server* server, Channel* chan, std::string& appliedModes, std::string& modeParams);

	//flags 
	static bool isIgnoredCommand(const std::string& cmd);

public: 
	Command();
	Command(const Command &copy);
	Command &operator=(const Command &copy);
	~Command();

	// Logic
	void parse(const std::string &line);
	void execute(Client* client, Server* server);

	std::string	getCmdName() const;

	// command executions:
	void	executePASS(Client* client, Server* server);
	void	executeNICK(Client* client, Server* server);
	void	executeUSER(Client* client, Server* server);
	void	executeJOIN(Client* client, Server* server);
	void	executePRIVMSG(Client* client, Server* server);
	void	executePART(Client* client, Server* server);
	void	executeQUIT(Client* client, Server* server);
	void	executeKICK(Client* client, Server* server);
	void	executeMODE(Client* client, Server* server);
	void	executeINVITE(Client* client, Server* server);
	void	executeTOPIC(Client* client, Server* server);
};

#endif
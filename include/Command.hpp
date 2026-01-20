#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "Client.hpp"
#include <string>
#include <vector>

class Server;

class Command {
private:
	std::string					_rawLine;
	std::string					_cmdName;
	std::vector<std::string>	_params;
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
};

#endif
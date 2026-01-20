#include "Command.hpp"

Command::Command() : 
	_rawLine(""),
	_cmdName("") {}

Command::Command(const Command &copy);
Command::Command &operator=(const Command &copy);
Command::~Command();
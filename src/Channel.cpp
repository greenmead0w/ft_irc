#include "include/Channel.hpp"
#include <iostream>


Channel::Channel() :
	_name(""),
	_userLimit(0),
	_modeInviteOnly(false),
	_modeTopicRestricted(true),
	_hasPassword(false),
	_hasLimit(false) {}

Channel::Channel(std::string name) :
	_name(name),
	_userLimit(0),
	_modeInviteOnly(false),
	_modeTopicRestricted(true),
	_hasPassword(false),
	_hasLimit(false) {}

Channel::~Channel() {}


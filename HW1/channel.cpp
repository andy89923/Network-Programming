#include <cstring>
#include <map>
using namespace std;


#include "Channel.h"
#include "User.h"

map<string, int> channel_map;
Channel channels[MAXCONN];

Channel::Channel() {
	this -> clear();
}

void Channel::clear() {
	this -> v.clear();
	this -> used  = 0;
	this -> name  = "";
	this -> topic = "";
}

void Channel::setTopic(string topic) {
	this -> topic = name;
}

void Channel::setName(string name) {
	this -> name = name;
	this -> used = 1;
}

void Channel::push_usr(User& client) {
	this -> v.insert(&client);
}

void Channel::pop_usr(User& client) {
	this -> v.erase(v.find(&client));
}

bool Channel::isUsed() const {
	return this -> used;
}

int Channel::get_num_usr() const {
	return (int) v.size();
}

string Channel::getName() const {
	return this -> name;
}

string Channel::getTopic() const {
	return this -> topic;
}



#include <cstring>
#include <map>
#include <iostream>
#include <sstream>
#include <iomanip>
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
	this -> topic = topic;
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

void Channel::send_message(string source, string message) {
	string s = ":" + source + " PRIVMSG #" + this -> name + " :";
	s += message;

	char const *pchar = s.c_str(); 
	for (auto i : this -> v) if (i -> getName() != source) {
		send(i -> getFD(), pchar, s.length(), 0);
	}
}

bool Channel::isUsed() const {
	return this -> used;
}

int Channel::get_num_usr() const {
	return (int) v.size();
}

string Channel::getUsers() const {
	string s = "";
	for (auto i : this -> v) {
		s = s + (i -> getName()) + " ";
	}
	return s;
}

string Channel::getName() const {
	return this -> name;
}

string Channel::getTopic() const {
	if (this -> topic == "") 
		return "No topic is set";
	return this -> topic;
}
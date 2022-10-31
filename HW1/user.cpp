#include "User.h"

#include <arpa/inet.h>
#include <string>
using namespace std;


User::User() {
	this -> fd_num = -1;
}

void User::init() {
	this -> fd_num = -1;
	this -> name = "";
}

void User::setIP(in_addr ip) {
	this -> ip = ip;
}

void User::setPort(int port) {
	this -> port = port;
}

void User::setFD(int fd_num) {
	this -> fd_num = fd_num;
}

void User::setName(string name) {
	this -> name = name;
}


int User::getFD() const {
	return this -> fd_num;
}

bool User::isUsed() const {
	return (this -> fd_num > 0);
}

string User::getName() const {
	return this -> name;
}


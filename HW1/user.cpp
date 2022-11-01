#include "User.h"
#include "Handler.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>
#include <iostream>
using namespace std;

int num_clients;

User::User() {
	this -> fd_num = -1;
}

void User::init() {
	this -> fd_num = -1;
	this -> name = "";
	this -> registered = 0;
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

// Nickname
void User::setName(string name) {
	this -> name = name;
	this -> registered |= 1;

	if (this -> registered == 3) this -> welcome_message();
}

void User::setUser(string  username, 
					string hostname, 
					string servername, 
					string realname    ) {

	this -> username   = username;
	this -> hostname   = hostname;
	this -> servername = servername;
	this -> realname   = realname;

	this -> registered |= 2;

	if (this -> registered == 3) this -> welcome_message();
}

void User::welcome_message() {
	string now = "";
	string name = this -> name;

	now += Handler::getDataFormat(001, name) + "Welcome to the minimized IRC daemon!\n";
	now += Handler::getDataFormat(251, name) + "There are " + to_string(num_clients);
	now += " users and 0 invisible on 1 server\n";

	string tmp = Handler::getDataFormat(372, name);

	now += Handler::getDataFormat(375, name) + "- mircd Message of the day -\n";
	now += tmp                               + "-              \n";
	now += tmp                               + "-  Hello World!\n";
	now += tmp                               + "-              \n";
	now += Handler::getDataFormat(376, name) + "End of message of the day\n";

	// send
	char const *pchar = now.c_str(); 
	send(this -> getFD(), pchar, now.length(), 0);

	this -> registered = 5;
}

int  User::getFD()     const { return this -> fd_num;          }
bool User::isRegist()  const { return this -> registered >= 5; }
bool User::isUsed()    const { return this -> fd_num > 0;      }
string User::getName() const { return this -> name;            }


#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;

#include "Handler.h"
#include "User.h"
#include "IRCError.h"

// ":<servername> <code> <user_nick> "
string Handler::getDataFormat(int code, string name) {
	stringstream ss;
	ss << server_constants::SERVER_PREFIX;
	ss << setw(3) << setfill('0') << code;
	ss << ' ' << name << " ";
	return ss.str();
}

// Command: USER
//    Parameters: <username> <hostname> <servername> <realname>
int Handler::set_user_info(char** rec, User& client, int cnt) {
	if (cnt < 5 || cnt > 5) {
		cerr << "USER command error!\n";
		return 1;
	}

	client.setUser(rec[1], rec[2], rec[3], rec[4]);
}

// Command: NICK
//    Parameters: <nickname> [ <hopcount> ]
int Handler::change_user_name(char** rec, User& client, int cnt) {
	if (cnt < 2) {
		// "ERR_NONICKNAMEGIVEN"
		IRCERROR::sent_error("ERR_NONICKNAMEGIVEN", client);	
		return 1;
	}
	if (cnt > 2) {
		// Name contaion white space
		return 1;
	}
	client.setName(rec[1]);
	return 0;
}

void Handler::list_users(User& client) {
	stringstream ss;

	ss << Handler::getDataFormat(392, client.getName());
	ss << ":UserID                         Terminal  Host\n";

	string tmp = Handler::getDataFormat(393, client.getName());
	for (auto i : clients) {
		if (!i.isUsed()) continue;
		ss << tmp << ":";

		ss << setw(31) << left << setfill(' ') << i.getName();
		ss << setw(10) << left << setfill(' ') << "-";
		ss << inet_ntoa(i.getIP()) << "\n";
	}
	ss << Handler::getDataFormat(394, client.getName());
	ss << ":End of users\n";

	string now = ss.str();
	char const *pchar = now.c_str(); 
	send(client.getFD(), pchar, now.length(), 0);
}



void Handler::handle(char** rec, User& client, int cnt) {
	if (strcmp(rec[0], "USER") == 0) {
		Handler::set_user_info(rec, client, cnt);
		return;
	}
	if (strcmp(rec[0], "NICK") == 0) {
		Handler::change_user_name(rec, client, cnt);
		return;
	}

	// Cheak whether user register or not
	// if (!client.isRegist()) {
	// 	IRCERROR::sent_error("ERR_NOTREGISTERED", client);
	// 	return;
	// }


	if (strcmp(rec[0], "USERS") == 0) {
		cerr << "USERS command found!\n";
		Handler::list_users(client);
		return;
	}


	// Command not found
	cerr << rec[0] << " command not found!\n";

}
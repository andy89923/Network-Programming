#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;

#include "Handler.h"
#include "User.h"
#include "IRCError.h"

// :<servername> <code> <user_nick> :
string Handler::getDataFormat(int code, string name) {
	stringstream ss;
	ss << server_constants::SERVER_PREFIX;
	ss << std::setw(3) << setfill('0') << code;
	ss << ' ' << name << " :";
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
	if (!client.isRegist()) {
		IRCERROR::sent_error("ERR_NOTREGISTERED", client);
		return;
	}


	if (strcmp(rec[0], "USERS") == 0) {
		cerr << "USERS command found!\n";
		
		return;
	}


	// Command not found
	cerr << rec[0] << " command not found!\n";

}
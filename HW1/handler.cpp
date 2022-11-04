#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <set>
using namespace std;

#include "Handler.h"
#include "User.h"
#include "IRCError.h"
#include "Channel.h"

bool Handler::send_data(string s, User& client) {
	char const *pchar = s.c_str(); 
	send(client.getFD(), pchar, s.length(), 0);
}

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
		IRCERROR::sent_error("ERR_NEEDMOREPARAMS", client);
		return 1;
	}

	client.setUser(rec[1], rec[2], rec[3], rec[4]);
}

// Command: NICK
//    Parameters: <nickname> [ <hopcount> ]
int Handler::change_user_name(char** rec, User& client, int cnt) {
	if (cnt < 2) {
		IRCERROR::sent_error("ERR_NONICKNAMEGIVEN", client);	
		return 1;
	}
	if (cnt > 2) { // Name contaion white space
		return 1;
	}
	client.setName(rec[1]);
	if (all_user_name.find(rec[1]) != all_user_name.end()) {
		IRCERROR::sent_error("ERR_NICKCOLLISION", client);	
		return 1;
	}
	all_user_name.insert(rec[1]);
	return 0;
}


// Command: JOIN
//    Parameters: <channel>
void Handler::join_channel(char** rec, User& client, int cnt) {
	if (cnt < 2) {
		IRCERROR::sent_error("ERR_NEEDMOREPARAMS", client);
		return;
	}
	if (cnt > 2) {
		return;
	}
	string name = rec[1];
	if (channel_map.find(name) == channel_map.end()) {

		// TODO

		return;
	}

}

// Command: USERS
void Handler::list_users(User& client) {
	stringstream ss;

	ss << Handler::getDataFormat(392, client.getName());
	ss << ":";
	ss << setw(31) << left << "UserID";
	ss << setw(10) << left << "Terminal";
	ss << "Host\n";

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

	Handler::send_data(ss.str(), client);
}

// Command: LIST
// :mircd 321 test23 Channel :Users Name
// :mircd 323 test23 :End of Liset
void Handler::list_channel(User& client) {
	stringstream ss;

	ss << Handler::getDataFormat(321, client.getName());
	ss << "Channel :Users Name\n";

	string tmp = Handler::getDataFormat(322, client.getName());
	for (int i = 0; i < MAXCONN; i++) if (channels[i].isUsed()) {
		ss << tmp << " #" << channels[i].getName() << " ";
		ss << channels[i].get_num_usr() << " :";
		ss << channels[i].getTopic() << '\n';
	}

	ss << Handler::getDataFormat(323, client.getName());
	ss << ":End of Liset\n";

	Handler::send_data(ss.str(), client);
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

	if (strcmp(rec[0], "PING") == 0) { // Pong
		Handler::send_data("PONG <null>\n", client);
		return;
	}
	if (strcmp(rec[0], "USERS") == 0) {
		Handler::list_users(client);
		return;
	}

	if (strcmp(rec[0], "TOPIC") == 0) {
		// User must in channel
		// TOPIC #abc :This is Topic
		return;
	}
	if (strcmp(rec[0], "LIST") == 0) {
		Handler::list_channel(client);
		return;
	}
	if (strcmp(rec[0], "NAMES") == 0) {
		return;
	}
	if (strcmp(rec[0], "JOIN") == 0) {
 		// :mircd 331 CCC #B :No topic is set
 		// :mircd 353 CCC #B :<u> <u>
 		// :mircd 366 CCC #B :End of Names List
		return;
	}
	if (strcmp(rec[0], "PART") == 0) {
		return;
	}
	if (strcmp(rec[0], "PRIVMSG") == 0) {
		return;
	}


	if (strcmp(rec[0], "QUIT") == 0) {
		
		return;
	}


	// Command not found
	cerr << rec[0] << " command not found!\n";

}
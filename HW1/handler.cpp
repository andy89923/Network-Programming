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
#include "Global.h"

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

// Command: USER <username> <hostname> <servername> <realname>
int Handler::set_user_info(char** rec, User& client, int cnt) {
	if (cnt < 5 || cnt > 5) {
		cerr << "USER command error!\n";
		IRCERROR::sent_error("ERR_NEEDMOREPARAMS", client);
		return 1;
	}

	client.setUser(rec[1], rec[2], rec[3], rec[4]);
}

// Command: NICK <nickname>
int Handler::change_user_name(char** rec, User& client, int cnt) {
	if (cnt < 2) {
		IRCERROR::sent_error("ERR_NONICKNAMEGIVEN", client);	
		return 1;
	}
	if (cnt > 2) { return 1; }

	client.setName(rec[1]);
	if (all_user_name.find(rec[1]) != all_user_name.end()) {
		IRCERROR::sent_error("ERR_NICKCOLLISION", client);	
		return 1;
	}
	all_user_name.insert(rec[1]);
	return 0;
}

// Command: PART <Channel>
void Handler::leave_channel(char** rec, User& client, int cnt) {
	if (cnt < 2) {
		IRCERROR::sent_error("ERR_NEEDMOREPARAMS", client);
		return;
	}
	if (cnt > 2) { return; }

	string name = rec[1];
	if (name[0] == '#') name.erase(name.begin());

	if (channel_map.find(name) == channel_map.end()) {
		IRCERROR::sent_error("ERR_NOSUCHCHANNEL", client);
		return;
	}

	if (client.getChat() != name || client.getChat() == "") {
		IRCERROR::sent_error("ERR_NOTONCHANNEL", client);
		return;
	}

	int idx = channel_map[name];

	channels[idx].pop_usr(client);
	client.joinChat("");

	stringstream ss;
	ss << ":" << client.getName() << " PART :#" << name << '\n';
	Handler::send_data(ss.str(), client);
}

// Command: JOIN <channel>
void Handler::join_channel(char** rec, User& client, int cnt) {
	if (cnt < 2) {
		IRCERROR::sent_error("ERR_NEEDMOREPARAMS", client);
		return;
	}
	if (cnt > 2) { return; }

	string name = rec[1];
	if (name[0] == '#') name.erase(name.begin());

	if (channel_map.find(name) == channel_map.end()) {
		for (int i = 0; i < MAXCONN; i++) if (!channels[i].isUsed()) {
			channels[i].clear();
			channels[i].setName(name);
			channel_map[name] = i;
			break;
		}
		
	}
	int idx = channel_map[name];

	channels[idx].push_usr(client);
	client.joinChat(name);

	stringstream ss;

	ss << ":" << client.getName() << " JOIN #" << name << "\n";
	ss << getDataFormat(331, client.getName()) << "#" << name << " :";
	ss << channels[idx].getTopic() << '\n';
	ss << getDataFormat(353, client.getName()) << "#" << name << " :";
	ss << channels[idx].getUsers() << '\n';
	ss << getDataFormat(366, client.getName()) << "#" << name << " :";
	ss << "End of Names List\n";

	Handler::send_data(ss.str(), client);
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

// Command: TOPIC <channel> [<topic>]
void Handler::setTopic(char** rec, User& client, int cnt) {
	if (cnt < 3) {
		IRCERROR::sent_error("ERR_NEEDMOREPARAMS", client);
		return;
	}
	string name = rec[1]; name.erase(name.begin());
	if (client.getChat() != name || client.getChat() == "") {
		IRCERROR::sent_error("ERR_NOTONCHANNEL", client);
		return;
	}
	string topic = rec[2]; topic.erase(topic.begin());
	for (int i = 3; i < cnt; i++) topic = topic + " " + rec[i];

	int idx = channel_map[name];
	channels[idx].setTopic(topic);

	stringstream ss;
	ss << server_constants::SERVER_PREFIX;
	ss << " 332 " << client.getName() << " " << rec[1] << ":" << topic << '\n';

	Handler::send_data(ss.str(), client);
}

// Command: NAMES [<channel>]
void Handler::list_channel_users(char** rec, User& client, int cnt) {
	stringstream ss;

	for (int i = 0; i < MAXCONN; i++) if (channels[i].isUsed()) {
		bool filter = (cnt <= 1);
		
		for (int j = 1; j < cnt; j++) {
			string yy = rec[j];
			if (yy[0] == '#') yy.erase(yy.begin());
			if (channels[i].getName() == rec[j]) filter = true;
		}
		if (!filter) continue;

		string name = channels[i].getName();
		ss << getDataFormat(353, client.getName()) << "#" << name << " :";
		ss << channels[i].getUsers() << '\n';
		ss << getDataFormat(366, client.getName()) << "#" << name << " :";
		ss << "End of Names List\n";
	}
	Handler::send_data(ss.str(), client);
}

// Command: PRIVMSG <channel> <message>
void Handler::send_message(char** rec, User& client, int cnt) {
	if (cnt < 3) {
		IRCERROR::sent_error("ERR_NEEDMOREPARAMS", client);
		return;
	}
	string name = rec[1]; name.erase(name.begin());
	if (channel_map.find(name) == channel_map.end()) {
		IRCERROR::sent_error("ERR_NOSUCHNICK", client);
		return;
	}
	int idx = channel_map[name];

	string message = rec[2];
	for (int i = 3; i < cnt; i++) message = message + " " + rec[i];

	channels[idx].send_message(client.getName(), message);
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
		Handler::setTopic(rec, client, cnt);
		return;
	}
	if (strcmp(rec[0], "LIST") == 0) {
		Handler::list_channel(client);
		return;
	}
	if (strcmp(rec[0], "NAMES") == 0) {
		Handler::list_channel_users(rec, client, cnt);
		return;
	}
	if (strcmp(rec[0], "JOIN") == 0) {
		Handler::join_channel(rec, client, cnt);
		return;
	}
	if (strcmp(rec[0], "PART") == 0) {
		Handler::leave_channel(rec, client, cnt);
		return;
	}
	if (strcmp(rec[0], "PRIVMSG") == 0) {
		Handler::send_message(rec, client, cnt);
		return;
	}


	if (strcmp(rec[0], "QUIT") == 0) {	
		return;
	}


	// Command not found
	cerr << rec[0] << " command not found!\n";

}
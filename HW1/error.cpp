#include <map>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
using namespace std;

#include "IRCError.h"
#include "User.h"
#include "Handler.h"

map<string, int> IRCERROR::error_code;
map<int, string> IRCERROR::error_info;

void IRCERROR::init_error() {
	init_error_3();
	init_error_4();
	init_error_info();
}

void IRCERROR::init_error_info() {
	error_info[331] = " :No topic is set";
	error_info[401] = " :No such nick/channel";
	error_info[403] = " :No such channel";
	error_info[451] = " :You have not registered";
	error_info[436] = " :Nickname collision KILL";
	error_info[461] = " :Not enought parameters";
	error_info[431] = " :No nickname given";
	error_info[421] = " :Unknown command";
	error_info[411] = " :No recipient given (PRIVMSG)";
	error_info[412] = " :No text to send";
	error_info[442] = " :You are not on that channel";
}

void IRCERROR::sent_error_with_channel(string type, User client, string chat) {
	cerr << "ERROR " << type << '\n';

	int code = error_code[type];
	string ss = server_constants::SERVER_PREFIX;

	ss += to_string(code) + " " + client.getName() + " #" + chat;
	ss += error_info[code];
	ss += "\n";

	// send
	char const *pchar = ss.c_str(); 
	send(client.getFD(), pchar, ss.length(), 0);
}

void IRCERROR::sent_error_with_command(string type, User client, string command) {
	cerr << "ERROR " << type << '\n';

	int code = error_code[type];
	string ss = server_constants::SERVER_PREFIX;

	ss += to_string(code) + " " + client.getName() + " " + command;
	ss += error_info[code];
	ss += "\n";

	// send
	char const *pchar = ss.c_str(); 
	send(client.getFD(), pchar, ss.length(), 0);
}

void IRCERROR::sent_error(string type, User client) {
	cerr << "ERROR " << type << '\n';

	int code = error_code[type];
	string ss = server_constants::SERVER_PREFIX;

	ss += to_string(code) + " " + client.getName() + error_info[code];
	ss += "\n";

	// send
	char const *pchar = ss.c_str(); 
	send(client.getFD(), pchar, ss.length(), 0);
}


void IRCERROR::init_error_3() {
	error_code["RPL_LISTSTART"]  = 321;
	error_code["RPL_LIST"]       = 322;
	
	error_code["RPL_LISTEND"]    = 323;
	error_code["RPL_NOTOPIC"]    = 331;
	
	error_code["RPL_TOPIC"]      = 331;
	error_code["RPL_NAMREPLY"]   = 353;

	error_code["RPL_ENDOFNAMES"] = 366;
	error_code["RPL_MOTD"]       = 372;
	
	error_code["RPL_MOTDSTART"]  = 375;
	error_code["RPL_ENDOFMOTD"]  = 376;

	error_code["RPL_USERSSTART"] = 392;
	error_code["RPL_USERS"]      = 393;
	
	error_code["RPL_ENDOFUSERS"] = 394;
}

void IRCERROR::init_error_4() {
	error_code["ERR_NOSUCHNICK"]      = 401;
	error_code["ERR_NOSUCHCHANNEL"]   = 403;

	error_code["ERR_NORECIPIENT"]     = 411;
	error_code["ERR_NOTEXTTOSEND"]    = 412;

	error_code["ERR_UNKNOWNCOMMAND"]  = 421;
	error_code["ERR_NONICKNAMEGIVEN"] = 431;

	error_code["ERR_NICKCOLLISION"]   = 436;
	error_code["ERR_NOTONCHANNEL"]    = 442;

	error_code["ERR_NOTREGISTERED"]   = 451;
	error_code["ERR_NEEDMOREPARAMS"]  = 461;	
}

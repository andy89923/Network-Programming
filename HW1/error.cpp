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
	error_info[451] = " :You have not registered";
}

void IRCERROR::sent_error(string type, User client) {
	cerr << "ERROR " << type << '\n';

	int code = error_code[type];
	string ss = server_constants::SERVER_PREFIX;

	ss += to_string(code) + error_info[code];
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

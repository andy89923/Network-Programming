#include <map>
#include <iostream>
using namespace std;

#include "IRCError.h"

map<string, int> IRCERROR::error_code;

void IRCERROR::init_error() {
	init_error_3();
	init_error_4();
}

void IRCERROR::sent_error(string type) {
	cerr << error_code[type] << '\n';

	
}
/*
/*
(321) RPL_LISTSTART
(322) RPL_LIST

(323) RPL_LISTEND
(331) RPL_NOTOPIC

(332) RPL_TOPIC
(353) RPL_NAMREPLY

(366) RPL_ENDOFNAMES
(372) RPL_MOTD

(375) RPL_MOTDSTART
(376) RPL_ENDOFMOTD

(392) RPL_USERSSTART
(393) RPL_USERS

(394) RPL_ENDOFUSERS
*/
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

/*
(401) ERR_NOSUCHNICK
(403) ERR_NOSUCHCHANNEL

(411) ERR_NORECIPIENT
(412) ERR_NOTEXTTOSEND

(421) ERR_UNKNOWNCOMMAND
(431) ERR_NONICKNAMEGIVEN

(436) ERR_NICKCOLLISION
(442) ERR_NOTONCHANNEL

(451) ERR_NOTREGISTERED
(461) ERR_NEEDMOREPARAMS
*/
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

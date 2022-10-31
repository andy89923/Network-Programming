
#include "Handler.h"
#include "User.h"
#include "IRCError.h"
#include <cstring>
#include <iostream>
using namespace std;

int Handler::change_user_name(char** rec, User& client, int cnt) {
	if (cnt < 2) {
		// "ERR_NONICKNAMEGIVEN"
		IRCERROR::sent_error("ERR_NONICKNAMEGIVEN", client);	
		
		return 1;
	}
	client.setName(rec[1]);

	cerr << "Change " << rec[1] << '\n';
	return 0;
}

void Handler::handle(char** rec, User& client, int cnt) {
	if (strcmp(rec[0], "NICK") == 0) {
		cerr << "NICK command found!\n";

		Handler::change_user_name(rec, client, cnt);
	}	


}
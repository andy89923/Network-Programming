#ifndef User_H_INCLUDED
#define User_H_INCLUDED

#include <arpa/inet.h>
#include <string>
#include <set>
using namespace std;

#include "Global.h"

class User {
protected:
	int fd_num;
	int port;

	in_addr ip;
	string name;  // Nickname

	string username, hostname, servername, realname;
	string cha_name;

	int registered;


public:
	User();

	void init();

	void setIP(in_addr);
	void setPort(int);
	void setFD(int);
	void setName(string);

	void setUser(string, string, string, string);
	void joinChat(string);

	void welcome_message();

	bool inChat() const;
	bool isUsed() const;
	bool isRegist() const;
	string getName() const;
	string getChat() const;
	int getFD() const;
	in_addr getIP() const;
};

extern int num_clients; 
extern User clients[MAXCONN];
extern set<string> all_user_name;

#endif // User_H_INCLUDED
#ifndef User_H_INCLUDED
#define User_H_INCLUDED

#include <arpa/inet.h>
#include <string>

using namespace std;

extern int num_clients;

class User {
protected:
	int fd_num;
	int port;

	in_addr ip;
	string name;  // Nickname

	string username, hostname, servername, realname;

	int registered;

public:
	User();

	void init();

	void setIP(in_addr);
	void setPort(int);
	void setFD(int);
	void setName(string);

	void setUser(string, string, string, string);

	void welcome_message();

	bool isUsed() const;
	bool isRegist() const;
	string getName() const;
	int getFD() const;
};



#endif // User_H_INCLUDED
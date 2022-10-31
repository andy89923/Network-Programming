#ifndef User_H_INCLUDED
#define User_H_INCLUDED

#include <arpa/inet.h>
#include <string>

using namespace std;

class User {
protected:
	int fd_num;
	int port;

	in_addr ip;
	std::string name;

public:
	User();

	void init();

	void setIP(in_addr);
	void setPort(int);
	void setFD(int);
	void setName(string);


	bool isUsed() const;
	string getName() const;
	int getFD() const;
};



#endif // User_H_INCLUDED
#ifndef Channel_H_INCLUDED
#define Channel_H_INCLUDED

#include <cstring>
#include <string>
#include <set>
#include <vector>
using namespace std;

#include "User.h"
#include "Global.h"

class Channel {
protected:
	int used;
	
	string name, topic;
	set<User*> v;

public:
	Channel();

	void clear();

	void setTopic(string);
	void setName(string);

	void push_usr(User&);
	void pop_usr(User&);

	bool isUsed() const;
	int get_num_usr() const;
	string getName() const;
	string getTopic() const;
};

extern map<string, int> channel_map;
extern Channel channels[MAXCONN];

#endif // Channel_H_INCLUDED
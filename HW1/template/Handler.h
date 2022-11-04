#ifndef Handler_H_INCLUDED
#define Handler_H_INCLUDED


#include "User.h"

class Handler {

protected:

	static int set_user_info(char**, User&, int);
	static int change_user_name(char**, User&, int);
	static void list_users(User&);
	static void list_channel(User&);
	static void join_channel(char**, User&, int);
	static void setTopic(char**, User&, int);
	static void list_channel_users(char**, User&, int);
	static void leave_channel(char**, User&, int);
	static void send_message(char**, User&, int);

public:

	static void handle(char**, User&, int);

	static string getDataFormat(int, string);

	static bool send_data(string, User&);
};


namespace server_constants {

    const string SERVER_PREFIX = ":ctfang ";
}



#endif // Handler_H_INCLUDED
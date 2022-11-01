#ifndef IRCERROR_H_INCLUDED
#define IRCERROR_H_INCLUDED

#include <map>
#include <string>
#include "User.h"
using namespace std;


class IRCERROR {

public:

    static map<string, int> error_code;
    static map<int, string> error_info;

    static void init_error();

	static void init_error_3();
	static void init_error_4();
	static void init_error_info();

	static void sent_error(string type, User who);
};



#endif // IRCERROR_H_INCLUDED
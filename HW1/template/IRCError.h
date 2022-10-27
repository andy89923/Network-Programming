#ifndef IRCERROR_H_INCLUDED
#define IRCERROR_H_INCLUDED

#include <map>
#include <string>
using namespace std;


class IRCERROR {

public:

    static map<string, int> error_code;

    static void init_error();

	static void init_error_3();
	static void init_error_4();

	static void sent_error(string type);
};



#endif // IRCERROR_H_INCLUDED
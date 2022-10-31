#ifndef Handler_H_INCLUDED
#define Handler_H_INCLUDED


#include "User.h"

class Handler {

protected:

	static int change_user_name(char**, User&, int);

public:

	static void handle(char**, User&, int);
};

namespace server_constants {

    const string SERVER_PREFIX = "ctfang: ";
}



#endif // Handler_H_INCLUDED
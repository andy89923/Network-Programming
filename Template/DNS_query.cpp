#include <iostream>
#include <netdb.h>
#include <arpa/inet.h>
using namespace std;

int main(int argc, char *argv[]) {
	
	struct hostent *hptr;

	hptr = gethostbyname("www.nycu.edu.tw");
	cout << "Original = "<< hptr -> h_name << '\n';

	char **pptr;
	for (pptr = hptr -> h_aliases; *pptr != NULL; pptr++) {
		cout << *pptr << '\n';
	}
	char str[INET_ADDRSTRLEN];

	if (hptr -> h_addrtype == AF_INET) {
		for (pptr = hptr -> h_addr_list; *pptr != NULL; pptr++) {

			cout << "\tIP = ";
			cout << inet_ntop(hptr -> h_addrtype, hptr -> h_addr, str, sizeof(str)) << '\n';

			// in_addr_t now = inet_addr(hptr -> h_addr);
			// cout << inet_ntoa(now) << '\n';
		}
	}
	return 0;
}
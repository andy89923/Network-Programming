#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <sstream>
#include <iomanip>
using namespace std;

#define MAX 2048
#define MAXFILE 1010

#define SEG 10

struct files {

	int recv[SEG];

	string data[SEG];
	string name;

};

files f[MAXFILE];
char buf[MAX];


// /server <path-to-store-files> <total-number-of-files> <port>
int main(int argc, char* argv[]) {
	
	int num_files = 10;

	for (int i = 0; i < num_files; i++) {
		std::stringstream ss;
		ss << std::setw(6) << std::setfill('0') << i;
		f[i].name = ss.str();

		cout << f[i].name << '\n';

		for (int j = 0; j < SEG; j++) {
			f[i].data[j] = "";
			f[i].recv[j] =  0;
		}
	}
	cout << "======= TEST ========\n";

	string raw = "00000105abcdefg";

	string fil = string(buf);


	cout << fil << '\n';	

	memset(buf, 0, sizeof(buf));

	strncpy(buf, raw.c_str() + 6, 2);

	int idx = atoi(buf);

	cout << idx << '\n';


	return 0;
}
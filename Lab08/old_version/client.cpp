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

#define SEG 2048
#define SENDSIZE 256

struct files {

	int send[SEG];

	string data[SEG];
	string name;

};

files f[MAXFILE];
char buf[MAX], tmp[MAX];

// /client <path-to-read-files> <total-number-of-files> <port> <server-ip-address>
int main(int argc, char* argv[]) {
	if (argc < 4) {
		cout << "Usage: /client <path-to-read-files> <total-number-of-files> <port> <server-ip-address>\n";
		exit(1);
	}
	memset(buf, 0, sizeof(buf));

	string root_path = argv[1];
	int num_files = atoi(argv[2]);

	for (int i = 0; i < num_files; i++) {
		std::stringstream ss;
		ss << std::setw(6) << std::setfill('0') << i;
		f[i].name = ss.str();

		ifstream file(root_path + "/" + f[i].name);
		
		int eof = 0;
		for (int j = 0; j < SEG; j++) {
			if (!eof) {
				file.read(buf, SENDSIZE);
				size_t extracted = file.gcount();
				if (extracted < SENDSIZE) {
					eof = 1;
				}
			}
			f[i].data[j] = string(buf);
			f[i].send[j] =  0;

			memset(buf, 0, SENDSIZE);
		}
	}

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	struct sockaddr_in server_id;	
	server_id.sin_family = AF_INET;
	server_id.sin_port   = atoi(argv[3]);


	for (int i = 0; i < num_files; i++) {
		for (int j = 0; j < SEG; j++) {
			memset(buf, 0, sizeof(buf));

			stringstream ss;
			ss << f[i].name << setw(4) << setfill('0') << j;
			ss << f[i].data[j];
			
			strcpy (buf, ss.str().c_str());
				

			// cout << "Send " << buf << '\n';
			// sendto(s, buf, SENDSIZE + 8, 0, (struct sockaddr*) &server_id, sizeof(server_id));	

			f[i].send[j] = 1;
		}
	}



	return 0;
}
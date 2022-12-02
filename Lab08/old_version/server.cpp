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

void udp_socket(int& sock, sockaddr_in& server_id, int port) {
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (sock < 0) {
        cout << "Error on socket init!\n";
        exit(1);
    }    
    const int opt = true;
    socklen_t optlen = sizeof(opt);
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, optlen);
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, optlen);
    
    bzero(&server_id, sizeof(server_id));
	server_id.sin_family = AF_INET;
	server_id.sin_port = htons(port);

	int r = bind(sock, (sockaddr*) &server_id, sizeof(sockaddr_in));
	if (r < 0) {
		cout << "Error on bind!\n";
		exit(1);
	}
}

#define SEG 2048
#define SENDSIZE 256

struct files {
	int recv[SEG];

	string data[SEG];
	string name;
};

files f[MAXFILE];
char buf[MAX], tmp[MAX];

string root_path;
int num_files;

void handle_file(int sig) {
	for (int i = 0; i < num_files; i++) {
		ofstream file(root_path + "/" + f[i].name);

		for (int j = 0; j < SEG; j++) if (f[i].recv[j]) {
			file << f[i].data[j];
		}
		file.close();
	}
	exit(0);
}

// /server <path-to-store-files> <total-number-of-files> <port>
int main(int argc, char* argv[]) {
	if (argc < 4) {
		cout << "Usage: /server <path-to-store-files> <total-number-of-files> <port>\n";
		exit(1);
	}
	root_path = argv[1];
	num_files = atoi(argv[2]);

	for (int i = 0; i < num_files; i++) {
		stringstream ss;
		ss << std::setw(6) << std::setfill('0') << i;
		f[i].name = ss.str();

		for (int j = 0; j < SEG; j++) {
			f[i].data[j] = "";
			f[i].recv[j] =  0;
		}
	}

	int sock;
	int listen_port = atoi(argv[3]);
	struct sockaddr_in server_id;

	// socket 
	udp_socket(sock, server_id, listen_port);


	struct sockaddr_in client_id;
	bzero(&client_id, sizeof(client_id));
	socklen_t csinlen = sizeof(client_id);
	
	signal(SIGINT, handle_file);

	while (true) {
		int rlen;
		if( (rlen = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr*) &client_id, &csinlen)) < 0) {
			perror("recvfrom");
			break;
		}

		string raw = string(buf); 

		// file_name
		memset(tmp, 0, sizeof(tmp));
		strncpy(tmp, raw.c_str(), 6);
		int now_file = atoi(tmp);

		// idx
		memset(tmp, 0, sizeof(tmp));
		strncpy(tmp, raw.c_str() + 10, 4);
		int now_idx = atoi(tmp);

		// data
		memset(tmp, 0, sizeof(tmp));
		strncpy(tmp, raw.c_str() + 10, rlen - 10);
		string now_data = string(tmp);

		f[now_file].data[now_idx] = now_data;
		f[now_file].recv[now_idx] = 1;

		memset(buf, 0, sizeof(buf));

		// sendto(s, buf, rlen, 0, (struct sockaddr*) &csin, sizeof(csin));
	}

	return 0;
}
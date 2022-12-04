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
#include "header.h"
using namespace std;

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

struct files {
	int recv[SEG];
	int leng[SEG];
	int max_indx, cnt;

	string name;
	char data[SEG][MAX_SENDSIZE];
};

files f[MAXFILE];
char buf[MAX];

string root_path;
int num_files;

void output_file(int i) {
	ofstream file(root_path + f[i].name, ios::out | ios::binary);

	for (int j = 0; j < f[i].max_indx; j++) if (f[i].recv[j]) {
		file.write(f[i].data[j], f[i].leng[j]);
	}
	file.close();
}

uint64_t cal_checksum(char* c, int len) {
	uint64_t checksum = 0, now = 0;

	for (int j = 0; j < len; j++) {
		now <<= 8;	
		now += (uint64_t) c[j];

		if (j != 0 && j % 8 == 7) {
			checksum = checksum ^ now;
			now = 0;
		}
	}
	if (len % 8 != 0) {
		for (int j = 0; j < 8 - (len % 8); j++) {
			now <<= 8;	
		}
	}
	checksum = checksum ^ now;
	return checksum;
}

void init() {
	for (int i = 0; i < num_files; i++) {
		stringstream ss;
		ss << setw(6) << setfill('0') << i;
		
		f[i].name = ss.str();
		f[i].max_indx = -1;
		f[i].cnt = 0;

		for (int j = 0; j < SEG; j++) {
			memset(f[i].data[j], 0, MAX_SENDSIZE);
			f[i].recv[j] =  0;
		}
	}
}

void send_ack(int sock, sockaddr_in& client_id, int now_file, int now_indx) {
	char ack[16];
	
	memset(ack, 0, sizeof(ack));
	memcpy(ack, &now_file, sizeof(now_file));
	memcpy(ack + sizeof(now_file), &now_indx, sizeof(now_indx));

	uint64_t checksum = cal_checksum(ack, 8);
	memcpy(ack + 8, &checksum, sizeof(checksum));

	sendto(sock, ack, sizeof(ack), 0, (struct sockaddr*) &client_id, sizeof(client_id));
	sendto(sock, ack, sizeof(ack), 0, (struct sockaddr*) &client_id, sizeof(client_id));
}

// /server <path-to-store-files> <total-number-of-files> <port>
int main(int argc, char* argv[]) {
	if (argc < 4) {
		cout << "Usage: /server <path-to-store-files> <total-number-of-files> <port>\n";
		exit(1);
	}
	root_path = argv[1]; 
	root_path = root_path + "/";
	num_files = atoi(argv[2]);
		
	num_files = NUM_FILES;

	init();

	int sock;
	int listen_port = atoi(argv[3]);
	struct sockaddr_in server_id;

	// socket 
	udp_socket(sock, server_id, listen_port);

	struct sockaddr_in client_id;
	bzero(&client_id, sizeof(client_id));
	socklen_t csinlen = sizeof(client_id);
	
	#define FILE_NAME_LEN 6
	#define FILE_IDXS_LEN 4
	#define DATA_LENG_LEN 4

	int sum = 0;
	while (true) {
		memset(buf, 0, sizeof(buf));

		int rlen;
		if( (rlen = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr*) &client_id, &csinlen)) < 0) {
			perror("recvfrom");
			break;
		}
		int offset = 0;
		int now_file, now_indx, max_indx, dat_leng;
		
		// file name
		memcpy(&now_file, buf + offset, FILE_NAME_LEN);
		offset += FILE_NAME_LEN;

		// idx
		memcpy(&now_indx, buf + offset, FILE_IDXS_LEN);
		offset += FILE_IDXS_LEN;

		// max idx
		memcpy(&max_indx, buf + offset, FILE_IDXS_LEN);
		offset += FILE_IDXS_LEN;

		// data length
		memcpy(&dat_leng, buf + offset, DATA_LENG_LEN);
		offset += DATA_LENG_LEN;

		// checksum
		uint64_t check_sum;
		memcpy(&check_sum, buf + offset, sizeof(check_sum));
		offset += sizeof(check_sum);

		memcpy(f[now_file].data[now_indx], buf + offset, rlen - offset);

		uint64_t now_checksum = cal_checksum(f[now_file].data[now_indx], dat_leng);

		if (check_sum != now_checksum) continue;
		send_ack(sock, client_id, now_file, now_indx);

		if (f[now_file].recv[now_indx]) continue;
		
		f[now_file].recv[now_indx] = 1;
		f[now_file].max_indx = max_indx;
		f[now_file].leng[now_indx] = dat_leng;

		sum += dat_leng;
		
		f[now_file].cnt += 1;

		if (f[now_file].max_indx == f[now_file].cnt) {
			output_file(now_file);
			f[now_file].max_indx = 0x3f3f3f3f;
		}
	}

	return 0;
}

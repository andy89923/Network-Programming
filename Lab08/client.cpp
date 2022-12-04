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

struct files {
	int send[SEG], max_indx;
	int leng[SEG];

	string name;

	char data[SEG][MAX_SENDSIZE];

	uint64_t checksum[SEG];
};

files f[MAXFILE];
char buf[MAX];

int num_files;
string root_path;

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
	int sum = 0;
	for (int i = 0; i < num_files; i++) {
		std::stringstream ss;
		ss << std::setw(6) << std::setfill('0') << i;
		f[i].name = ss.str();

		string now_path = root_path + f[i].name;

		ifstream file;
		file.open(now_path, ios::binary);
		
		int cnt = 0;
		for (int j = 0; j < SEG; j++) {
			memset(buf, 0, MAX);
			
			int len = file.readsome(buf, SENDSIZE);
			if (len == 0) break;

			cnt += 1;
			memcpy(f[i].data[j], buf, SENDSIZE);

			f[i].leng[j] = len;
			f[i].send[j] =  0;
			f[i].checksum[j] = cal_checksum(buf, f[i].leng[j]);

			sum += f[i].leng[j];
		}
		f[i].max_indx = cnt;

		file.close();

		// cout << now_path << " " << f[i].max_indx << ' ' << sum << '\n';
	}
}

#define FILE_NAME_LEN 6
#define FILE_IDXS_LEN 4
#define DATA_LENG_LEN 4
#define ACK_LEN       16

void send_data(int sock, int now_file, int now_indx, sockaddr_in &server_id) {
	int offset = 0;
	memcpy(buf + offset, &now_file, FILE_NAME_LEN);
	offset += FILE_NAME_LEN;

	// idx
	memcpy(buf + offset, &now_indx, FILE_IDXS_LEN);
	offset += FILE_IDXS_LEN;

	// max_idx
	memcpy(buf + offset, &f[now_file].max_indx, FILE_IDXS_LEN);
	offset += FILE_IDXS_LEN;

	// length
	memcpy(buf + offset, &f[now_file].leng[now_indx], FILE_IDXS_LEN);
	offset += FILE_IDXS_LEN;

	// checksum
	memcpy(buf + offset, &f[now_file].checksum[now_indx], 8);
	offset += 8;

	memcpy(buf + offset, f[now_file].data[now_indx], f[now_file].leng[now_indx]);
	offset += f[now_file].leng[now_indx];

	sendto(sock, buf, offset, 0, (struct sockaddr*) &server_id, sizeof(server_id));	
}

void check_ack(int sock, sockaddr_in &server_id) {
	char ack[ACK_LEN];
	memset(ack, 0, sizeof(ack));
	
	socklen_t servlen = sizeof(server_id);
	int rlen;

	if ( (rlen = recvfrom(sock, ack, sizeof(ack), 0, (struct sockaddr*) &server_id, &servlen) ) < 0) {
        return;
    }
    int now_file, now_indx;
    uint64_t checksum;
    uint64_t now_checksum = cal_checksum(ack, 8);

    memcpy(&now_file, ack, sizeof(now_file));
    memcpy(&now_indx, ack + 4, sizeof(now_indx));
    memcpy(&checksum, ack + 8, sizeof(checksum));

    if (checksum != now_checksum) return;

    if (now_file < num_files && now_indx < f[now_file].max_indx) {
    	f[now_file].send[now_indx] = 1;

    	// cout << "ACK " << now_file << ' ' << now_indx << '\n';
    }
}

// /client <path-to-read-files> <total-number-of-files> <port> <server-ip-address>
int main(int argc, char* argv[]) {
	if (argc < 4) {
		cout << "Usage: /client <path-to-read-files> <total-number-of-files> <port> <server-ip-address>\n";
		exit(1);
	}
	memset(buf, 0, sizeof(buf));

	root_path = argv[1]; 
	root_path = root_path + "/";
	num_files = atoi(argv[2]);
	
	num_files = NUM_FILES;

	init();

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = TIMEOUT;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	struct sockaddr_in server_id;	
	server_id.sin_family = AF_INET;
	server_id.sin_port   = htons(atoi(argv[3]));
	inet_pton(AF_INET, argv[4], &server_id.sin_addr);

	while (true) {
		int send_cnt = 0;

		for (int i = 0; i < num_files; i++) {
			for (int j = 0; j < f[i].max_indx; j++) {
				if (f[i].send[j]) continue;

				memset(buf, 0, sizeof(buf));
				send_cnt += 1;

				send_data(sock, i, j, server_id);

				check_ack(sock, server_id);
			}
		}
		
		if (send_cnt == 0) break;
	}

	return 0;
}

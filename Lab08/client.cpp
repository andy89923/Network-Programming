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

#define SEG 2048
#define MAX 2048
#define MAXFILE 1010
#define MAX_SENDSIZE 256
#define SENDSIZE 200

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
			if (!file.eof()) {
				memset(buf, 0, MAX_SENDSIZE);
				cnt += 1;

				int len = file.readsome(buf, SENDSIZE);

				memcpy(f[i].data[j], buf, SENDSIZE);

				f[i].leng[j] = len;
				f[i].send[j] =  0;
				f[i].checksum[j] = cal_checksum(buf, f[i].leng[j]);

				sum += f[i].leng[j];
			}
			else
				f[i].send[j] = 1;
		}
		f[i].max_indx = cnt;

		file.close();

		// cout << now_path << " " << f[i].max_indx << ' ' << sum << '\n';
	}
}

#define FILE_NAME_LEN 6
#define FILE_IDXS_LEN 4
#define DATA_LENG_LEN 4

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
	
	init();

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	struct sockaddr_in server_id;	
	server_id.sin_family = AF_INET;
	server_id.sin_port   = htons(atoi(argv[3]));
	inet_pton(AF_INET, argv[argc-1], &server_id.sin_addr);

	for (int i = 0; i < num_files; i++) {
		for (int j = 0; j < f[i].max_indx; j++) {
			memset(buf, 0, sizeof(buf));

			int cnt = 5;
			while(cnt--) {
				send_data(sock, i, j, server_id);
				sleep(0.1);
			}
			sleep(0.2);
		}
	}

	return 0;
}

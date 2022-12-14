#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <iomanip>
using namespace std;

#include "header.h"

char buf[MAX_BUF];
File f[NUM_FILE];

string rot_path;
int num_file;

void load_file() {
	char tmp[7];
	for (int i = 0; i < num_file; i++) {
		int sum = 0;

		snprintf(tmp, 7, "%06d", i);
		f[i].name = tmp;

		string now_path = rot_path + f[i].name;

		ifstream file;
		file.open(now_path, ios::binary);
		
		int cnt = 0;
		for (int j = 0; j < SEG; j++) {
			int len = file.readsome(buf, MAX_SED);
			if (len == 0) break;

			int now_fid = i * 10000 + j;

			memcpy(f[i].data[j] + 0, &now_fid, 4);
			memcpy(f[i].data[j] + 8, buf, len);

			sum += len;
			cnt += 1;

			f[i].leng[j] = len + 8;
		}
		f[i].max_indx = cnt;
		for (int j = 0; j < cnt; j++) {
			int now_idx = cnt * 10000 + f[i].leng[j];

			memcpy(f[i].data[j] + 4, &now_idx, 4);
		}
		file.close();
	}
}

void init() {
	memset(buf, 0, sizeof(buf));

	rot_path = rot_path + "/";
	for (int i = 0; i < num_file; i++) f[i].clear();

	load_file();
}

char ack[ACK_LEN];
int ack_cnt = 0;

void check_ack(int sock, sockaddr_in &server_id) {
	while (true) {
		memset(ack, 0, sizeof(ack));
		
		socklen_t servlen = sizeof(server_id);
		int rlen;

		if ((rlen = recvfrom(sock, ack, sizeof(ack), 0, (struct sockaddr*) &server_id, &servlen)) < 0) {
	        return;
	    }
	    if (rlen == 0) break;

	    int now_idx, nxt_idx;
	    memcpy(&now_idx, ack + 0, 4);

	    int now_file = now_idx / 10000;
	    int now_indx = now_idx % 10000;

	    if (now_file == 1001 && now_indx == 1001) exit(0);

		f[now_file].send[now_indx] = 1;
	}
}

// /client <path-to-read-files> <total-number-of-files> <port> <server-ip-address>
int main(int argc, char* argv[]) {
	if (argc < 4) {
		cout << "Usage: /client <path-to-read-files> <total-number-of-files> <port> <server-ip-address>\n";
		exit(1);
	}

	rot_path = argv[1]; 
	num_file = atoi(argv[2]);

	init();

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = TIMEOUT;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	int recv_siz = RECV_SIZ;
	setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &recv_siz, sizeof(recv_siz));

	struct sockaddr_in server_id;
	server_id.sin_family = AF_INET;
	server_id.sin_port   = htons(atoi(argv[3]));
	inet_pton(AF_INET, argv[4], &server_id.sin_addr);
	socklen_t addr_len = sizeof(server_id);

	int flag = 0;
	do {

		flag = 0;
		for (int i = 0; i < num_file; i++) {
			for (int j = 0; j < f[i].max_indx; j++) if (!f[i].send[j]) {
				flag = 1;

				sendto(sock, f[i].data[j], f[i].leng[j], 0,  (struct sockaddr*) &server_id, addr_len);	
			}	
		}
		check_ack(sock, server_id);
		
	} while (flag);

	return 0;
}
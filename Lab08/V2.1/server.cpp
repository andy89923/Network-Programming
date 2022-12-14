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

void init() {
	memset(buf, 0, sizeof(buf));

	char tmp[7];
	for (int i = 0; i < num_file; i++) {
		f[i].clear();

		snprintf(tmp, 7, "%06d", i);
		f[i].name = tmp;
	}
}

inline void output_file(int i) {
	ofstream file(rot_path + f[i].name, ios::out | ios::binary);

	for (int j = 0; j < f[i].max_indx; j++) if (f[i].send[j]) {
		file.write(f[i].data[j], f[i].leng[j]);
	}
	file.close();
}

char ack[ACK_LEN];
void send_ack(int& sock, sockaddr_in& client_id, int now_file, int now_indx) {
	memset(ack, 0, sizeof(ack));

	int now_idx = now_file * 10000 + now_indx;

	memcpy(ack + 0, &now_idx, 4);

	int t = 3;
	while (t--)
		sendto(sock, ack, 4, 0, (struct sockaddr*) &client_id, sizeof(client_id));
}

// /server <path-to-store-files> <total-number-of-files> <port>
int main(int argc, char* argv[]) {
	if (argc < 4) {
		cout << "Usage: /server <path-to-store-files> <total-number-of-files> <port>\n";
		exit(1);
	}
	rot_path = argv[1]; 
	rot_path = rot_path + "/";
	num_file = atoi(argv[2]);

	init();

	int sock;
	int listen_port = atoi(argv[3]);
	struct sockaddr_in server_id;

	// socket 
	udp_socket(sock, server_id, listen_port);

	struct sockaddr_in client_id;
	bzero(&client_id, sizeof(client_id));
	socklen_t csinlen = sizeof(client_id);

	int output_file_cnt = 0;
	while (true) {
		memset(buf, 0, sizeof(buf));

		int rlen;
		if ((rlen = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr*) &client_id, &csinlen)) < 0) {
			perror("recvfrom");
			break;
		}

		int now_fid, now_idx;
		memcpy(&now_fid, buf + 0, 4);
		memcpy(&now_idx, buf + 4, 4);
		
		int now_file = now_fid / 10000;
		int now_indx = now_fid % 10000;
		int max_indx = now_idx / 10000;
		int dat_leng = now_idx % 10000;
		
		send_ack(sock, client_id, now_file, now_indx);	
		if (f[now_file].send[now_indx]) continue;

		f[now_file].send[now_indx] = 1;
		f[now_file].leng[now_indx] = dat_leng - 8;
		memcpy(f[now_file].data[now_indx], buf + 8, dat_leng - 8);

		f[now_file].max_indx = max_indx;
		f[now_file].cnt_indx += 1;

		if (f[now_file].max_indx == f[now_file].cnt_indx) {
			output_file(now_file);
			f[now_file].max_indx = 0x3f3f3f3f;
			output_file_cnt += 1;
		}
		if (output_file_cnt == num_file) break;
	}
	while (true)
		send_ack(sock, client_id, 1001, 1001);


	return 0;
}

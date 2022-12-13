#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <iomanip>
#include <signal.h>
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

void output_file(int i) {
	ofstream file(rot_path + f[i].name, ios::out | ios::binary);

	for (int j = 0; j < f[i].max_indx; j++) if (f[i].send[j]) {
		file.write(f[i].data[j], f[i].leng[j]);
	}
	file.close();
}

char ack[ACK_LEN];
int sock;
struct sockaddr_in client_id;
void send_ack(int now_file, int now_indx) {
	memset(ack, 0, sizeof(ack));

	int now_idx = now_file * 10000 + now_indx;

	memcpy(ack + 0, &now_idx, 4);
	memcpy(ack + 4, &now_idx, 4);

	// cout << "Send ack: " << now_file << ' ' << now_indx << '\n';
	int t = 2;
	while (t--)
		sendto(sock, ack, 9, 0, (struct sockaddr*) &client_id, sizeof(client_id));
}

void send_all_ack() {
	for (int i = 0; i < num_file; i++) {
		int t = (f[i].max_indx == 0x3f3f3f3f ? 32 : f[i].max_indx);

		for (int j = 0; j < t; j++) if (f[i].send[j]) {
			send_ack(i, j);
		}
	}
}

void alarm_handle(int sig) {
    send_all_ack();
    alarm(2);
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

	int listen_port = atoi(argv[3]);
	struct sockaddr_in server_id;

	// socket 
	udp_socket(sock, server_id, listen_port);

	bzero(&client_id, sizeof(client_id));
	socklen_t csinlen = sizeof(client_id);

	int output_file_cnt = 0;
    signal(SIGALRM, alarm_handle);
    alarm(1);
	while (true) {
		memset(buf, 0, sizeof(buf));

		int rlen;
		if ((rlen = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr*) &client_id, &csinlen)) < 0) {
			perror("recvfrom");
			break;
		}

		uint64_t checksum, tmp = 0;
		memcpy(&checksum, buf + 8, 8);
		memcpy(buf + 8, &tmp, 8);

		tmp = cal_checksum(buf, rlen);
		if (tmp != checksum) continue;

		int now_fid, now_idx;
		memcpy(&now_fid, buf + 0, 4);
		memcpy(&now_idx, buf + 4, 4);
		
		int now_file = now_fid / 10000;
		int now_indx = now_fid % 10000;
		int max_indx = now_idx / 10000;
		int dat_leng = now_idx % 10000;

		if (dat_leng != rlen) continue;
		
		if (f[now_file].send[now_indx]) {
		    send_ack(now_file, now_indx);
			// send_all_ack(sock, client_id);
			continue;
		}
		send_ack(now_file, now_indx);	

		f[now_file].send[now_indx] = 1;
		f[now_file].leng[now_indx] = dat_leng - 16;
		memcpy(f[now_file].data[now_indx], buf + 16, dat_leng - 16);

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
		send_ack(1001, 1001);


	return 0;
}

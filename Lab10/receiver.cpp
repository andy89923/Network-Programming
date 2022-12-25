#include <bits/stdc++.h>
#include "header.h"
using namespace std;

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

	for (int j = 0; j < f[i].max_indx; j++) {
		file.write(f[i].data[j], f[i].leng[j]);
	}
	file.close();
}

// /server <path-to-store-files> <total-number-of-files> <broadcast-address>
int main(int argc, char const *argv[]) {
	rot_path = argv[1]; 
	rot_path = rot_path + "/";
	num_file = atoi(argv[2]);

	init();

	int sock;
	raw_socket(sock);
 
	struct sockaddr_in server_id;
	bzero(&server_id, sizeof(server_id));

	server_id.sin_family = AF_INET;
	server_id.sin_port = htons(APP_PORT);
	bind(sock, (sockaddr*) &server_id, sizeof(sockaddr_in));

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
		cout << rlen << " recv something!\n";
		cout << buf << '\n';

		char* data = buf + sizeof(iphdr) + sizeof(udphdr);
		
		struct iphdr *ip_header = (iphdr*) buf;
		struct udphdr *udp_header = (struct udphdr*) (buf + sizeof(struct iphdr)); 

		cout << "IP CHECK " << ip_header -> check << '\n';
		cout << "UDP Check " << udp_header -> check << '\n';
		for (int k = 0; k < rlen; k++) cout << buf[k];
		cout << '\n';

		int now_fid, now_idx;
		memcpy(&now_fid, data + 0, 4);
		memcpy(&now_idx, data + 4, 4);
		
		int now_file = now_fid / 10000;
		int now_indx = now_fid % 10000;
		int max_indx = now_idx / 10000;
		int dat_leng = now_idx % 10000;

		cout << now_file << ' ' << dat_leng << '\n';

		f[now_file].leng[now_indx] = dat_leng - 8;
		memcpy(f[now_file].data[now_indx], data + 8, dat_leng - 8);

		f[now_file].max_indx = max_indx;
		f[now_file].cnt_indx += 1;

		if (f[now_file].max_indx == f[now_file].cnt_indx) {
			output_file(now_file);
			f[now_file].max_indx = 0x3f3f3f3f;
			output_file_cnt += 1;
		}
		if (output_file_cnt == num_file) break;
	}
	return 0;
}

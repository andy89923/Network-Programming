#include <bits/stdc++.h>
#include "header.h"
using namespace std;

char buf[MAX_BUF];
File f[NUM_FILE];

string rot_path;
int num_file;

void construct_ip_hdr(struct iphdr* ip, int tot_len, char* source_ip, sockaddr_in addr) {
	ip -> version  = 4;
	ip -> ihl      = 5;
	ip -> tos      = 0;
	ip -> tot_len  = tot_len;
	ip -> id       = htonl(rand());
	ip -> frag_off = 0;
	ip -> ttl      = 64;
	// ip -> protocol = IPPROTO_UDP;
	ip -> protocol = 161;
	ip -> check    = 0;
	ip -> saddr    = addr.sin_addr.s_addr; // let sandbox get the broadcast packet
	ip -> daddr    = addr.sin_addr.s_addr;
}

void load_file(char* source_ip, sockaddr_in addr) {
	char tmp[7];

	for (int i = 0; i < num_file; i++) {
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

			memcpy(f[i].data[j] + sizeof(struct iphdr) + 0, &now_fid, 4);
			memcpy(f[i].data[j] + sizeof(struct iphdr) + 8, buf, len);

			cnt += 1;
			f[i].leng[j] = sizeof(struct iphdr) + len + 8;
		}
		file.close();

		f[i].max_indx = cnt;
		for (int j = 0; j < cnt; j++) {
			int now_idx = cnt * 10000 + f[i].leng[j];

			memcpy(f[i].data[j] + sizeof(struct iphdr) + 4, &now_idx, 4);

			// put IP layer data in
			int tot_len = f[i].leng[j] - sizeof(iphdr);
			construct_ip_hdr(f[i].data[j], tot_len, source_ip, addr);
			struct iphdr  *ip_header  = f[i].data[j];
			ip_header -> check = checksum((unsigned short*) datagram, tot_len);
		}
	}
}

void init(char* source_ip, char* address, struct sockaddr_in addr) {
	memset(buf, 0, sizeof(buf));

	rot_path = rot_path + "/";
	for (int i = 0; i < num_file; i++) f[i].clear();

	load_file(source_ip, addr);
}

// /client <path-to-read-files> <total-number-of-files> <broadcast-address>
int main(int argc, char const *argv[]) {
	rot_path = argv[1]; 
	num_file = atoi(argv[2]);

	struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(12345);
    addr.sin_addr.s_addr = inet_addr(address);

	init(argv[3], argv[3], addr);

	int sock;
	raw_socket(sock);

	int broadcast = 1;
  	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));


	for (int i = 0; i < num_file; i++) {
		for (int j = 0; j < f[i].max_indx; j++) {
			sendto(sock, f[i].data[j], f[i].leng[j], 0,  (struct sockaddr*) &addr, sizeof(addr));	
		}	
	}
	sleep(2);

	return 0;
}
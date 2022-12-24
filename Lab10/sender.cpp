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
	ip -> protocol = IPPROTO_UDP;
	// ip -> protocol = 161;
	ip -> check    = 0;
	ip -> saddr    = addr.sin_addr.s_addr; // let sandbox get the broadcast packet
	ip -> daddr    = addr.sin_addr.s_addr;
}

void construct_udp_hdr(udphdr *u, int udp_len, int source_port) {
    u -> source = htons(source_port);
    u -> dest   = htons(APP_PORT);
    u -> len    = htons(udp_len);
    u -> check  = 0;
}

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

// /client <path-to-read-files> <total-number-of-files> <broadcast-address>
int main(int argc, char const *argv[]) {
	rot_path = argv[1]; 
	num_file = atoi(argv[2]);

	init();

	int sock;
	raw_socket(sock);

	int broadcast = 1;
  	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));


	for (int i = 0; i < num_file; i++) {
		for (int j = 0; j < f[i].max_indx; j++) {
			memset(datagram, 0, sizeof(datagram));

			char* payload = datagram + sizeof(iphdr) + sizeof(udphdr);
			memcpy(data, f[i].data[j], f[i].leng[j]);

			struct iphdr  *ip_header  = (struct iphdr* ) (datagram);
			struct udphdr *udp_header = (struct udphdr*) (datagram + sizeof(struct iphdr)); 
    
   			int tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + f[i].leng[j];

			construct_ip_hdr(ip_header, tot_len, source_ip, addr);
			ip_header -> check = checksum((unsigned short*) datagram, ip_header -> tot_len);

			// UDP header
			int udp_data_len = sizeof(struct udphdr) + f[i].leng[j];
			construct_udp_hdr(udp_header, udp_data_len, 12345);

			int tmp_len = sizeof(struct udphdr) + f[i].leng[j];

			struct pshdr ps_hdr;
			ps_hdr.saddr = inet_addr(argv[3]);
			ps_hdr.daddr = inet_addr(argv[3]);
			ps_hdr.filler = 0;
			ps_hdr.protocol = IPPROTO_UDP;
			ps_hdr.len = htons(tmp_len);

			int siz = sizeof(struct pshdr) + udp_data_len;
			char* psgram = (char*) malloc(siz);

			memcpy(psgram, (char*) &ps_hdr, sizeof(struct pshdr));
			memcpy(psgram + sizeof(struct pshdr), udp_header, tmp_len);

			// UDP checksum
			udp_header -> check = checksum((unsigned short*) psgram, siz);

			sendto(sock, datagram, ip_header -> tot_len, 0,  (struct sockaddr*) &addr, sizeof(addr));	
		}	
	}
	sleep(2);

	return 0;
}
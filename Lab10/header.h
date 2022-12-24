#include <bits/stdc++.h>
#include <sys/socket.h>
#include <arpa/inet.h>    // inet_addr
#include <netinet/ip.h>   // Provides declarations for ip header
#include <netinet/udp.h>  // Provides declarations for udp header
using namespace std;

#define MAX_SED   1160
#define MAX_PKT   1300
#define MAX_BUF   1400
#define SEG         40
#define NUM_FILE  1005
#define TIMEOUT      5
#define ACK_LEN      5
#define RECV_SIZ 30000
#define MAX_MTU   1500
#define APP_PORT 12388

struct File {

	string name;

	int leng[SEG];
	int max_indx, cnt_indx; 

	char data[SEG][MAX_PKT];

	void clear() {
		for (int i = 0; i < SEG; i++) {
			leng[i] = 0;
			memset(data[i], 0, MAX_PKT);
		}
		max_indx = 0;
		cnt_indx = 0;
	}
};

int datagram[MAX_MTU];

void raw_socket(int& sock_r) {
    sock_r = socket(AF_INET, SOCK_RAW, 161);
    
    const int opt = true;
    socklen_t optlen = sizeof(opt);
    setsockopt(sock_r, SOL_SOCKET, SO_REUSEADDR, &opt, optlen);
    setsockopt(sock_r, SOL_SOCKET, SO_REUSEPORT, &opt, optlen);

    if (sock_r < 0) {
        cout << "Error on socket init!\n";
        exit(1);
    }
    int one = 1;
    if (setsockopt(sock_r, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        cout << "Fail to Set IP_HDRINCL\n";
        exit(2);
    } else
        cout << "Success to set IP_HDRINCL\n";
}

struct pshdr {
    u_int32_t saddr;
    u_int32_t daddr;
    u_int8_t filler;
    u_int8_t protocol;
    u_int16_t len;
};

unsigned short checksum(unsigned short *buf, int len){
    unsigned long sum = 0xffff;

    while (len > 1){
        sum += *buf;
        buf++;
        len -= 2;
    }
    if (len == 1) sum += *(unsigned char*) buf;

    sum = (sum & 0xffff) + (sum >> 16);
    sum = (sum & 0xffff) + (sum >> 16);
    return ~sum;
}
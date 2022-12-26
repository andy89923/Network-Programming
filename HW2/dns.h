#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
using namespace std;
typedef unsigned short ushort;

struct dnshdr {
    // short -> 2 byte
    unsigned short id;      // ID
    unsigned short flags;   // DNS Flags

    unsigned short qcnt;    // Question Count
    unsigned short acnt;    // Answer Count

    unsigned short auth;    // Authority RR
    unsigned short add;     // Additional RR

} __attribute__((packed));

struct query {
	char *name;
	string s;
	unsigned short qtype;
	unsigned short qclass;
};

struct additional_record {
	char name;
	unsigned short type;
	unsigned short udp_size;
	char rcode;
	char edns0;
	unsigned short z, data_len;
} __attribute__((packed));

void construct_dns_header(dnshdr* h, ushort id, bool isQuery, ushort qcnt, ushort acnt, ushort auth, ushort add = 1) {
	h -> id    = htons(id);
	if (isQuery)
		h -> flags = htons(0x0120);
	else
		h -> flags = htons(0x8500);

	// h -> qcnt  = htons(0x0001);
	// h -> acnt  = htons(0x0000);
	// h -> auth  = htons(0x0000);

	// HW only, no additional record
	h -> add   = htons(add);
}

void construct_add_rec(additional_record *ad) {
	ad -> name     = 0;
	ad -> type     = htons(41);
	ad -> udp_size = htons(4096);
	ad -> rcode    = 0;
	ad -> edns0    = 0;
	ad -> z        = 0;
	ad -> data_len = 0;
} 
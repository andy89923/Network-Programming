#include <bits/stdc++.h>
#include "dns.h"
using namespace std;

bool g_debug = false;

query parse_query(char* b, bool debug = false) {
	dnshdr* dns_header = (dnshdr*) b;

	if (debug != g_debug) g_debug = debug;

	query q;
	q.name   = (char*) (b + sizeof(dnshdr));

	memcpy(&q.qtype , b + sizeof(dnshdr) + strlen(q.name) + 1, sizeof(q.qtype));
	memcpy(&q.qclass, b + sizeof(dnshdr) + strlen(q.name) + 1 + sizeof(q.qtype), sizeof(q.qclass));

	q.qtype = ntohs(q.qtype);
	q.qclass = ntohs(q.qclass);

	if (g_debug) {
		cout << "\n";
		cout << "Get query\n";
		cout << "   id  : 0x" << hex << ntohs(dns_header -> id) << '\n';
		cout << "   name: ";
	}

	q.s = "";
	for (int i = 1; i < strlen(q.name); i++) {
		if (isprint(q.name[i])) {
			q.s += q.name[i];
		}
		else {
			q.s += ".";
		}
	}
	q.s += ".";

	if (g_debug) {
		cout << q.s << '\n';
		cout << "   type: " << q.qtype  << '\n';
		cout << "  class: " << q.qclass << '\n';
	}
	// qtype = 0xff -> ANY
	return q;
}

int forward_sock = -1;
struct sockaddr_in server_id;

int forward_dns(char* buf, char* rbuf, int rlen, sockaddr_in dns_server) {

	if (g_debug)
		cout << " --> forwarding...  ";

	if (forward_sock == -1) {
		udp_socket(forward_sock, server_id, 12000);
	}	

	sendto(forward_sock, buf, rlen, 0, (struct sockaddr*) &dns_server, sizeof(dns_server));

	struct sockaddr_in server_id, client_id;
	bzero(&client_id, sizeof(client_id));
	socklen_t csinlen = sizeof(client_id);

	int flen;
	if ((flen = recvfrom(forward_sock, rbuf, MAXBUF, 0, (struct sockaddr*) &client_id, &csinlen)) < 0) {
		perror("recvfrom");
		exit(1);
	}
	if (g_debug)
		cout << dec << "reply len = " << flen << "\n";

	return flen;
}

int handler(char* buf, char* rbuf, int rlen, vector<Zone>& v, sockaddr_in dns_server) {
	query q = parse_query(buf, true);
	// query q = parse_query(buf);

	string now = q.s;
	reverse(now.begin(), now.end());
	for (auto z: v) {
		string zone_name = z.name;
		reverse(zone_name.begin(), zone_name.end());

		if (strncmp(zone_name.c_str(), now.c_str(), zone_name.length()) == 0) {

			// cout << "Find match " << z.name << '\n';
			for (auto r: z.v) {
				// construct_dns_header(dnshdr* h, id, false, qcnt, acnt, auth, add) 



			}
			// not found a match record

			return 0;
		}
		// cout << now << " <-> " << zone_name << '\n';
	}
	// no-match
	// forward dns query

	return forward_dns(buf, rbuf, rlen, dns_server);
}
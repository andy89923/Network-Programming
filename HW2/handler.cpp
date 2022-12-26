#include <bits/stdc++.h>
#include "dns.h"
using namespace std;


query parse_query(char* b, bool debug = false) {
	dnshdr* dns_header = (dnshdr*) b;

	query q;
	q.name   = (char*) (b + sizeof(dnshdr));

	memcpy(&q.qtype , b + sizeof(dnshdr) + strlen(q.name) + 1, sizeof(q.qtype));
	memcpy(&q.qclass, b + sizeof(dnshdr) + strlen(q.name) + 1 + sizeof(q.qtype), sizeof(q.qclass));

	q.qtype = ntohs(q.qtype);
	q.qclass = ntohs(q.qclass);

	if (debug) {
		cout << "Get query\n";
		cout << "   id  : " << dns_header -> id << '\n';
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

	if (debug) {
		cout << q.s << '\n';
		cout << "   type: " << q.qtype  << '\n';
		cout << "  class: " << q.qclass << '\n';
	}
	// qtype = 0xff -> ANY
	return q;
}

void forward_dns() {
}

void handler(char* buf, int rlen, struct sockaddr_in& client_id, vector<Zone>& v) {
	query q = parse_query(buf, true);

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
			return;
		}
		// cout << now << " <-> " << zone_name << '\n';
	}
	// no-match
	// forward dns query



}
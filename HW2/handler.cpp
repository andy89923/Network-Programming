#include <bits/stdc++.h>
#include "dns.h"
using namespace std;

bool g_debug = false;

query parse_query(char* b, int& qlen) {
	dnshdr* dns_header = (dnshdr*) b;
	
	query q;
	q.name   = (char*) (b + sizeof(dnshdr));
	
	qlen = (int) strlen(q.name) + 1 + 4;

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

vector<Record*> ans, aut, add;

void init_handler(bool debug) {
	if (debug != g_debug) g_debug = debug;

	ans.clear();
	aut.clear();
	add.clear();
}

int add_records(char* c) {
	int tot_len = 0;

	for (auto i : ans) {
		tot_len += i -> push(c + tot_len);
	}
	for (auto i : aut) {
		tot_len += i -> push(c + tot_len);
	}
	for (auto i : add) {
		tot_len += i -> push(c + tot_len);
	}

	return tot_len;
}

int handler(char* buf, char* rbuf, int rlen, vector<Zone>& v, sockaddr_in dns_server, bool debug=false) {
	init_handler(debug);

	int qlen;
	query q = parse_query(buf, qlen);

	string now = q.s;
	reverse(now.begin(), now.end());
	for (auto& z: v) {
		string zone_name = z.name;
		reverse(zone_name.begin(), zone_name.end());

		if (strncmp(zone_name.c_str(), now.c_str(), zone_name.length()) == 0) {
			
			int flen = 0;

			for (auto& r : z.v) if (q.qtype == r.typ) {
				string now_r = r.name + "." + z.name;
				if (now_r == q.s) {
					if (g_debug) cout << "  => Found " << now_r << " <-> " << q.s << '\n';
					
					ans.push_back(&r);
					continue;
				}
				if (r.name == "") ans.push_back(&r);
			}
			if (ans.size() == 0) {
				aut.push_back(&z.v[0]); // SOA record
			} else {
				for (auto& r : z.v) if (r.typ == 2 && q.qtype != 2) {	
					aut.push_back(&r);
				}
			}
			if (q.qtype == 2 or q.qtype == 15) { // NS & MX
				for (auto& r : z.v) if (r.typ == 1) {
					for (auto i : ans) {
						string ss = r.name + "." + z.name;
						// cout << "-" << ss << "-  -" << (i -> data[(q.qtype == 2 ? 0 : 1)]) << "- \n";
						if (ss == (i -> data[(q.qtype == 2 ? 0 : 1)])) {
							add.push_back(&r);
						}
					}
				}
			}

			dnshdr* h = (dnshdr*) rbuf;
			dnshdr* dns_header = (dnshdr*) buf;
			
			construct_dns_header(h, ntohs(dns_header -> id), false, 1, ans.size(), aut.size(), add.size());
			flen += sizeof(dnshdr);

			memcpy(rbuf + flen, buf + sizeof(dnshdr), qlen);
			flen += qlen;

			flen += add_records(rbuf + flen);

			return flen;
		}
	}

	// no-match
	// forward dns query
	return forward_dns(buf, rbuf, rlen, dns_server);
}
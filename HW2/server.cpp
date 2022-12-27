#include <bits/stdc++.h>
#include "header.h"
#include "handler.cpp"
using namespace std;

char buf[MAXBUF], rbuf[MAXBUF];
struct sockaddr_in dns_server;
int listen_port;
vector<Zone> v;

void server_setup(const char* path) {

	ifstream file;
	file.open(path);

	string dns_ip;
	file >> dns_ip;

	init_message();

	// set forward dns server
	dns_server.sin_family = AF_INET;
	dns_server.sin_port   = htons(DNS_PORT);
	inet_pton(AF_INET, dns_ip.c_str(), &dns_server.sin_addr);

	v.clear();

	string now;
	while (file >> now) {

		int poi = now.find(',');

		string zone_name = now.substr(0, poi);
		string zone_path = now.substr(poi + 1, now.length() - poi);

		Zone now_zone;
		now_zone.setup(zone_name, zone_path);

		v.push_back(now_zone);

		zone_cnt += 1;
	}	
	file.close();

	cout << "=============================\n";
	cout << "==== DNS Server Settings ====\n";
	cout << "=============================\n";

	cout << "Forward DNS Server: " << dns_ip      << '\n';
	cout << "Number of Zones   : " << zone_cnt    << '\n';
	cout << "Listen on port    : " << listen_port << '\n';

	cout << "=============================\n";

	cout << "Zones:\n";
	cout << "----------------------------\n";
	for (int i = 0; i < zone_cnt; i++) {
		cout << "(" << i << ") " << v[i].name << '\n';
	}
	cout << "=============================\n";
}

// ./dns <port-number> <path/to/the/config/file>
int main(int argc, char const *argv[]) {
	if (argc < 3) {
		cout << "Usage: ./dns <port-number> <path/to/the/config/file>\n";
		exit(1);
	}
	system("clear");

	int sock;
	listen_port = atoi(argv[1]);
	struct sockaddr_in server_id, client_id;
	bzero(&client_id, sizeof(client_id));
	socklen_t csinlen = sizeof(client_id);

	udp_socket(sock, server_id, listen_port);
	
	server_setup(argv[2]);

	while (true) {
		memset( buf, 0, sizeof( buf));
		memset(rbuf, 0, sizeof(rbuf));

		int rlen;
		if ((rlen = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr*) &client_id, &csinlen)) < 0) {
			perror("recvfrom");
			break;
		}

		int flen = handler(buf, rbuf, rlen, v, dns_server, true);
		// int flen = handler(buf, rbuf, rlen, v, dns_server);

		sendto(sock, rbuf, flen, 0, (struct sockaddr*) &client_id, csinlen);
	}
	return 0;
}
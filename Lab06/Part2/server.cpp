#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <set>
#include <vector>
#include <sys/select.h>
#include <sys/time.h>
using namespace std;

#define BUFSIZE 10000
#define MAXCONN 1010

void tcp_socket(int& sock, sockaddr_in& server_id, int port) {
	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0) {
        cout << "Error on socket init!\n";
        exit(1);
    }
	const int opt = true;
    socklen_t optlen = sizeof(opt);
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, optlen);
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, optlen);

    bzero(&server_id, sizeof(server_id));

	server_id.sin_family      = PF_INET;
	server_id.sin_port        = htons(port);
	server_id.sin_addr.s_addr = htonl(INADDR_ANY);

	int r = bind(sock, (sockaddr*) &server_id, sizeof(sockaddr_in));
	if (r < 0) {
		cerr << "Error on bind!\n";
		exit(1);
	}
	r = listen(sock, MAXCONN);
}

char buf[BUFSIZE];
set<int> client_cmd;
set<int> client_sik;

fd_set now_set, all_set;
int max_fd;
int num_clients;

long long total_size;
struct timeval start, now_time;

void init() {
	client_cmd.clear();
	client_sik.clear();

	num_clients = 0;
	total_size = 0;
	gettimeofday(&start, NULL);

	FD_ZERO(&all_set);
	FD_ZERO(&now_set);

	max_fd = MAXCONN;
}

void handle(int now_cmd_sock) {
	gettimeofday(&now_time, NULL);

	stringstream ss;

	if (strncmp(buf, "/reset", 6) == 0) {	
		start = now_time;
		ss << start.tv_sec << '.' << start.tv_usec << " RESET " << total_size << "\n";
		total_size = 0;
	}
	if (strncmp(buf, "/ping", 5) == 0) {
		ss << now_time.tv_sec << '.' << now_time.tv_usec << " PONG\n";
	}
	if (strncmp(buf, "/report", 7) == 0) {
		double elp = (double) (now_time.tv_sec - start.tv_sec) + (now_time.tv_usec - start.tv_usec) * 1e-6;

		ss << now_time.tv_sec << '.' << now_time.tv_usec;
		ss << " REPORT " << total_size << " ";
		ss << elp << "s ";
		ss << 8.0 * total_size / 1000000.0 / (elp) << "Mbps\n";
	}
	if (strncmp(buf, "/clients", 8) == 0) {
		ss << now_time.tv_sec << '.' << now_time.tv_usec;
		ss << " CLIENTS " << num_clients << "\n";
	}

	string s = ss.str();
	char const *pchar = s.c_str(); 
	send(now_cmd_sock, pchar, s.length(), 0);
}

int main(int argc, char* argv[]) {
	init();

	int sock_cmd, port_cmd = atoi(argv[1]);
	int sock_sik, port_sik = port_cmd + 1;

	sockaddr_in server_id_cmd;
	sockaddr_in server_id_sik;

	tcp_socket(sock_cmd, server_id_cmd, port_cmd);
	tcp_socket(sock_sik, server_id_sik, port_sik);
	
	FD_SET(sock_cmd, &all_set);
	FD_SET(sock_sik, &all_set);

	gettimeofday(&start, NULL);

	int connect_fd;
	int info_len = sizeof(sockaddr);
	
	struct sockaddr_in client_id;
	bzero(&client_id, sizeof(client_id));

	signal(SIGPIPE, SIG_IGN);
	for ( ; ; ) {
		now_set = all_set;
		
		int num_ready = select(max_fd + 1, &now_set, NULL, NULL, NULL);

		if (FD_ISSET(sock_cmd, &now_set)) {
			connect_fd = accept(sock_cmd, (sockaddr*) &client_id, (socklen_t*) &info_len);

			client_cmd.insert(connect_fd);
			FD_SET(connect_fd, &all_set);

			// cerr << "New command connect\n";

			// string s = "Command!\n";
			// char const *pchar = s.c_str(); 
			// send(connect_fd, pchar, s.length(), 0);
		}

		if (FD_ISSET(sock_sik, &now_set)) {
			connect_fd = accept(sock_sik, (sockaddr*) &client_id, (socklen_t*) &info_len);

			client_sik.insert(connect_fd);
			FD_SET(connect_fd, &all_set);

			num_clients += 1;

			// cerr << "New sink connect\n";

			// string s = "Sink!\n";
			// char const *pchar = s.c_str(); 
			// send(connect_fd, pchar, s.length(), 0);
		}

		vector<int> removed; removed.clear();
		for (auto i : client_cmd) if (FD_ISSET(i, &now_set)) {
			memset(buf, 0, sizeof(buf));
			int read_len = read(i, buf, BUFSIZE);
			if (read_len == 0) {
				close(i);
				FD_CLR(i, &all_set);

				removed.push_back(i);
				continue;
			}
			handle(i);

			memset(buf, 0, sizeof(buf));
		}
		for (auto i : removed) client_cmd.erase(i);


		removed.clear();
		for (auto i : client_sik) if (FD_ISSET(i, &now_set)) {
			int read_len = read(i, buf, BUFSIZE);
			if (read_len == 0) {
				close(i);
				FD_CLR(i, &all_set);

				removed.push_back(i);
				continue;
			}
			total_size += read_len;
			memset(buf, 0, sizeof(buf));
		}
		for (auto i : removed) client_sik.erase(i);
	}


	return 0;
}
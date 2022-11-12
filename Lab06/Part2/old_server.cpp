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
#include <sys/select.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

#define BUFSIZE 10000
#define MAXCONN 1010

char buf[BUFSIZE];

int clients_cmd[MAXCONN];
int clients_sik[MAXCONN];
int num_clients;

int max_fd_cmd, max_po_cmd;
int max_fd_sik, max_po_sik;

fd_set rcv_set_cmd, all_set_cmd;
fd_set rcv_set_sik, all_set_sik;

int total_size;
long long start_time;

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

	server_id.sin_family = PF_INET;
	server_id.sin_addr.s_addr = htonl(INADDR_ANY);
	server_id.sin_port = htons(port);

	int r = bind(sock, (sockaddr*) &server_id, sizeof(sockaddr_in));
	if (r < 0) {
		cout << "Error on bind!\n";
		exit(1);
	}
	r = listen(sock, MAXCONN);
}

void select_init(int &socket_cmd, int &socket_sik) {
	FD_ZERO(&all_set_cmd);
	FD_ZERO(&rcv_set_cmd);

	FD_ZERO(&all_set_sik);
	FD_ZERO(&rcv_set_sik);

	max_fd_cmd = socket_cmd;
	max_po_cmd = -1;

	max_fd_sik = socket_sik;
	max_po_sik = -1;

	num_clients = 0;

	for (int i = 0; i < MAXCONN; i++) {
		clients_cmd[i] = -1;
		clients_sik[i] = -1;
	}

	FD_SET(socket_cmd, &all_set_cmd);
	FD_SET(socket_sik, &all_set_sik);
}


int main(int argc, char* argv[]) {
	
	int sock_cmd, port_cmd = atoi(argv[1]);
	int sock_sik, port_sik = port_cmd + 1;

	sockaddr_in server_id_cmd;
	sockaddr_in server_id_sik;

	tcp_socket(sock_cmd, server_id_cmd, port_cmd);
	tcp_socket(sock_sik, server_id_sik, port_sik);

	// Init
	select_init(sock_cmd, sock_sik);
	total_size = 0;
	start_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

	// Client
	int connect_fd;
	int info_len = sizeof(sockaddr);
	
	struct sockaddr_in client_id;
	bzero(&client_id, sizeof(client_id));

	signal(SIGPIPE, SIG_IGN);

	for (;;) {
		int now_sock, disconnect;
		do {

			rcv_set_cmd = all_set_cmd;
			int num_ready_cmd = select(max_fd_cmd + 1, &rcv_set_cmd, NULL, NULL, NULL);

			if (num_ready_cmd <= 0) break;

			// Command
			if (FD_ISSET(sock_cmd, &rcv_set_cmd)) {
				connect_fd = accept(sock_cmd, (sockaddr*) &client_id, (socklen_t*) &info_len);

				for (int i = 0; i < FD_SETSIZE; i++) {
					if (clients_cmd[i] < 0) {
						clients_cmd[i] = connect_fd;
						max_po_cmd = max(max_po_cmd, i);

						cerr << "New command server connection\n";
						break;
					}
					if (i == FD_SETSIZE - 1) {
						perror("Too many clients\n");
						exit(1);
					}
				}

				FD_SET(connect_fd, &all_set_cmd);
				max_fd_cmd = max(max_fd_cmd, connect_fd);

				num_ready_cmd -= 1;
				if (num_ready_cmd <= 0) break;
			}
			for (int i = 0; i <= max_po_cmd; i++) {
				if ((now_sock = clients_cmd[i]) < 0) continue;
			
				disconnect = 0;
				if (FD_ISSET(now_sock, &rcv_set_cmd)) {

					int read_len = read(now_sock, buf, BUFSIZE);
					if (read_len == 0) {
						close(now_sock);
						FD_CLR(now_sock, &all_set_cmd);

						clients_cmd[i] = -1;
						num_ready_cmd -= 1;

						continue;
					}

					cerr << "Cmd: " << buf << '\n';
					long long now_time;
					if (buf[0] == '/') {
						now_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
						if (strcmp(buf, "/reset")   == 0) {
							stringstream ss;
							ss << now_time << " RESET " << total_size << "\n";

							string s = ss.str();
							char const *pchar = s.c_str(); 
							send(clients_cmd[i], pchar, s.length(), 0);

							total_size = 0;
							start_time = now_time;
						}
						if (strcmp(buf, "/ping")    == 0) {
							cerr << "Ping found!\n";
							
							stringstream ss;
							ss << now_time << " PONG\n";
							string s = ss.str();
							char const *pchar = s.c_str(); 
							send(clients_cmd[i], pchar, s.length(), 0);
						}
						if (strcmp(buf, "/report")  == 0) {
							// <time> REPORT <counter-value> <elapsed-time>s <measured-megabits-per-second>Mbps\n
							stringstream ss;
							ss << now_time << " REPORT " << total_size << " ";
							ss << now_time - start_time << "s ";
							ss << 8.0 * total_size / 1000000.0 / (now_time - start_time) << "Mbps\n";

							string s = ss.str();
							char const *pchar = s.c_str(); 
							send(clients_cmd[i], pchar, s.length(), 0);
						}
						if (strcmp(buf, "/clients") == 0) {
							// <time> CLIENTS <number-of-connected-data-sink-connections>\n
							stringstream ss;
							ss << now_time << " CLIENTS " << num_clients << "\n";

							string s = ss.str();
							char const *pchar = s.c_str(); 
							send(clients_cmd[i], pchar, s.length(), 0);
						}
					}

					memset(buf, 0, sizeof(buf));

					num_ready_cmd -= 1;
					if (num_ready_cmd <= 0) break;
				}
			}

		} while (false);
		
		// Sink
		rcv_set_sik = all_set_sik;

		int num_ready_sik = select(max_fd_sik + 1, &rcv_set_sik, NULL, NULL, NULL);
		if (num_ready_sik <= 0) continue;

		if (FD_ISSET(sock_sik, &rcv_set_sik)) {
			connect_fd = accept(sock_sik, (sockaddr*) &client_id, (socklen_t*) &info_len);

			for (int i = 0; i < FD_SETSIZE; i++) {
				if (clients_sik[i] < 0) {
					clients_sik[i] = connect_fd;
					max_po_sik = max(max_po_sik, i);

					num_clients += 1;

					cerr << "New sink server connection\n";
					break;
				}
				if (i == FD_SETSIZE - 1) {
					perror("Too many clients\n");
					exit(1);
				}
			}

			FD_SET(connect_fd, &all_set_sik);
			max_fd_sik = max(max_fd_sik, connect_fd);

			num_ready_sik -= 1;
			if (num_ready_sik <= 0) continue;
		}

		// Sink
		for (int i = 0; i <= max_po_sik; i++) {
			if ((now_sock = clients_sik[i]) < 0) continue;
			
			disconnect = 0;
			if (FD_ISSET(now_sock, &rcv_set_sik)) {

				int read_len = read(now_sock, buf, BUFSIZE);
				if (read_len == 0) {
					close(now_sock);
					FD_CLR(now_sock, &all_set_sik);

					clients_sik[i] = -1;
					num_clients -= 1;
					num_ready_sik -= 1;
					if (num_ready_sik <= 0) break;

					continue;
				}
				cerr << "Sink:" << buf << '\n';
				
				total_size += read_len;
				memset(buf, 0, sizeof(buf));

				num_ready_sik -= 1;
				if (num_ready_sik <= 0) break;
			}
		}
	}
	return 0;
}
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <sys/select.h>
using namespace std;

#include "IRCError.h"
#include "User.h"
#include "Handler.h"
#include "Global.h"
#include "Channel.h"

char buf[BUFSIZE];

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

int max_fd, max_po;
fd_set rcv_set, all_set;

void select_init(int &socket) {
	FD_ZERO(&all_set);
	FD_ZERO(&rcv_set);

	max_fd = socket;
	max_po = -1;

	for (int i = 0; i < MAXCONN; i++) {
		clients[i].init();
	}

	FD_SET(socket, &all_set);
}

void server_init(int &sock) {
	IRCERROR::init_error();

	select_init(sock);
	num_clients = 0;
	all_user_name.clear();
	channel_map.clear();
}

int main(int argc, char* argv[]) {
	int sock, port = atoi(argv[1]);
	sockaddr_in server_id;

	tcp_socket(sock, server_id, port);

	server_init(sock);


	// Client
	int connect_fd;
	int info_len = sizeof(sockaddr);
	
	struct sockaddr_in client_id;
	bzero(&client_id, sizeof(client_id));

	for (;;) {
		rcv_set = all_set;

		int num_ready = select(max_fd + 1, &rcv_set, NULL, NULL, NULL);
		if (FD_ISSET(sock, &rcv_set)) {
			connect_fd = accept(sock, (sockaddr*) &client_id, (socklen_t*) &info_len);

			for (int i = 0; i < FD_SETSIZE; i++) {
				if (clients[i].getFD() < 0) {
					max_po = max(max_po, i);

					clients[i].setFD(connect_fd);
					clients[i].setIP(client_id.sin_addr);
					clients[i].setPort(ntohs(client_id.sin_port));

					num_clients += 1;

					// new connection
					cout << "New connect from " << inet_ntoa(clients[i].getIP()) << '\n';

					break;
				}
				if (i == FD_SETSIZE - 1) {
					perror("Too many clients\n");
					exit(1);
				}
			}

			FD_SET(connect_fd, &all_set);
			max_fd = max(max_fd, connect_fd);

			num_ready -= 1;
			if (num_ready <= 0) continue;
		}

		int now_sock, disconnect = 0;
		for (int i = 0; i <= max_po; i++) {
			if ((now_sock = clients[i].getFD()) < 0) continue;

			if (FD_ISSET(now_sock, &rcv_set)) {
				int read_len = read(now_sock, buf, BUFSIZE);
				if (read_len == 0) {
					close(now_sock);
					FD_CLR(now_sock, &all_set);
					
					clients[i].init();

					num_clients -= 1;
					disconnect = 1;
				}

				// buf
				// do something here
				if (!disconnect) {
					char s[] = " \n\r";
					char* recv[MAXARG];

					int cnt = 0;

					char *token;
					token = strtok(buf, s);

					while (token != NULL) {
						recv[cnt++] = token;
						token = strtok(NULL, s);
					}
					Handler::handle(recv, clients[i], cnt);
				}

				memset(buf, 0, sizeof(buf));

				num_ready -= 1;
				if (num_ready <= 0) break;
			}
		}
	}
	return 0;
}
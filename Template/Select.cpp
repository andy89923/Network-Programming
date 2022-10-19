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

#define BUFSIZE 10000
#define MAXCONN 10

char buf[BUFSIZE];

void tcp_socket(int& sock, sockaddr_in& server_id, int port) {
	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0) {
        cout << "Error on socket init!\n";
        exit(1);
    }
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

int clients[MAXCONN];
int max_fd, max_po;
fd_set rcv_set, all_set;

void select_init(int &socket) {
	FD_ZERO(&all_set);
	FD_ZERO(&rcv_set);

	max_fd = socket;
	max_po = -1;

	for (int i = 0; i < MAXCONN; i++) clients[i] = -1;

	FD_SET(socket, &all_set);
}

int main(int argc, char* argv[]) {

	int sock, port = atoi(argv[1]);
	sockaddr_in server_id;

	tcp_socket(sock, server_id, port);


	// Select 
	select_init(sock);


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
				if (i == FD_SETSIZE) {
					perror("Too many clients\n");
					exit(1);
				}

				if (clients[i] < 0) {
					clients[i] = connect_fd;
					max_po = max(max_po, i);
					break;
				}
			}

			FD_SET(connect_fd, &all_set);
			max_fd = max(max_fd, connect_fd);

			num_ready -= 1;
			if (num_ready <= 0) continue;
		}

		int now_sock;
		for (int i = 0; i <= max_po; i++) {
			if ((now_sock = clients[i]) < 0) continue;

			if (FD_ISSET(now_sock, &rcv_set)) {

				int read_len = read(now_sock, buf, BUFSIZE);
				if (read_len == 0) {
					// connection closed by client
					close(now_sock);
					FD_CLR(now_sock, &all_set);
					clients[i] = -1;
				}

				// buf
				// do something here


				num_ready -= 1;
				if (num_ready <= 0) break;
			}
		}
	}
	return 0;
}
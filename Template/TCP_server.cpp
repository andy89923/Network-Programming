#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
using namespace std;

void sig_chld(int sig_no) {
	
	pid_t pid;
	int  stat;

	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) { }
}

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
	r = listen(sock, SOMAXCONN);
}

void handle(int sock) {

}

int main(int argc, char* argv[]) {

	int sock;
	int listen_port = atoi(argv[1]);
	struct sockaddr_in server_id;

	// socket 
	tcp_socket(sock, server_id, listen_port);
	

	signal(SIGCHLD, sig_chld);

	char* cli_ip;
	int cli_port;

	int child_pid, connect_fd;
	int childer_len = sizeof(sockaddr);    // Input & Output
	
	struct sockaddr_in client_id;
	bzero(&client_id, sizeof(client_id));
	
	while (true) {
		if ((connect_fd = accept(sock, (sockaddr*) &client_id, (socklen_t*) &childer_len)) < 0) {
			if (errno == EINTR) continue;
			else {
				cout << "Connect error!\n";
				exit(1);
			}
		}
		cli_ip = inet_ntoa(client_id.sin_addr);
		cli_port = ntohs(client_id.sin_port); 
		
		cout << "New connection from " << cli_ip;
		cout << ":" << cli_port << "\n";

		if ((child_pid = fork()) == 0) {
			close(sock);

			handle(connect_fd);

			exit(0);
		}
	}

	return 0;
}
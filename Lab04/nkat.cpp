#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
using namespace std;
#define MAXARG 80
   
void sig_chld(int signo) {
	pid_t pid;
	int stat;

	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
	}
}

char* arg[MAXARG];

int handle(int sock) {
	
	dup2(sock, STDIN_FILENO);
	dup2(sock, STDOUT_FILENO);
	
	int r = execvp(arg[0], arg);

	if (r < 0) return 1;
}

int main(int argc, char const *argv[]) {
	
	if (argc < 3) {
		cout << "[Usage] ./nkat <port-numner> /path/to/an/external/program [optional arguments ...]\n";
		exit(0);
	}
	for (int i = 1; i < MAXARG; i++) arg[i] = NULL;



	int arg_cnt = 1;
	arg[0] = strdup(argv[2]);
	for (int i = 3; i < argc; i++) arg[arg_cnt++] = strdup(argv[i]);	



	int listen_port = atoi(argv[1]);

	// Info
	cout << "Listen port = " << listen_port << '\n';
	cout << "Running Program = ";
	for (int i = 0; i < arg_cnt; i++) cout << arg[i] << ' ';
	cout << '\n' << "Args = " << arg_cnt << '\n';



	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0) {
        cout << "Error on socket init!\n";
        exit(1);
    }

    struct sockaddr_in server_id, cliend_id;

	bzero(&server_id, sizeof(server_id));
	bzero(&cliend_id, sizeof(cliend_id));

	server_id.sin_family = PF_INET;
	server_id.sin_addr.s_addr = htonl(INADDR_ANY);
	server_id.sin_port = htons(listen_port);

	int r = bind(sock_fd, (sockaddr*) &server_id, sizeof(sockaddr_in));
	if (r < 0) {
		cout << "Error on bind!\n";
		exit(1);
	}
	r = listen(sock_fd, SOMAXCONN);

	signal(SIGCHLD, sig_chld);

	int cli_port, childpid;
	int connect_fd, childer_len = sizeof(sockaddr);
	char* cli_ip;

	while (true) {
		if ((connect_fd = accept(sock_fd, (sockaddr*) &cliend_id, (socklen_t*) &childer_len)) < 0) {
			if (errno == EINTR) continue;
			else {
				cout << "Connect error!\n";
				exit(1);
			}
		}

		cli_ip = inet_ntoa(cliend_id.sin_addr);
		cli_port = ntohs(cliend_id.sin_port); 

		cli_ip = inet_ntoa(cliend_id.sin_addr);
		cli_port = ntohs(cliend_id.sin_port); 
		
		cout << "New connection from " << cli_ip;
		cout << ":" << cli_port << "\n";

		if ((childpid = fork()) == 0) {
			close(sock_fd);

			int ret = handle(connect_fd);
			if (ret) {
				cerr << "Error on running command!\n";
			}
			exit(0);
		}
	}
	return 0;
}
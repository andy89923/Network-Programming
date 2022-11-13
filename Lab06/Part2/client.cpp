#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
using namespace std;

#define MAX 10010

char buf[MAX];
int socks[MAX];

int main(int argc, char* argv[]) {
	
	struct sockaddr_in server_id;

	bzero(&server_id, sizeof(server_id));
	server_id.sin_family = PF_INET;
	server_id.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_id.sin_port = htons(9999);


	int now_clients = 16;
	for (int i = 0; i < now_clients; i++) {
		socks[i] = socket(AF_INET, SOCK_STREAM, 0);
		connect(sock_fd, (struct sockaddr *) &server_id, sizeof(server_id));
	}

	for (;;) {
		for (int j = 0; j < now_clients; j++) {
			send(socks[j], buf, MAX, 0);
		}
	}


	return 0;
}
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <csignal>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
using namespace std;

#define MAX 10010

char buf[MAX];
int socks[MAX];

void handler(int s) {

	char message[] = { "/report\n" };
	send(socks[0], message, 8, 0);

	int read_len = read(socks[0], buf, 100);
	cout << buf << '\n';

	exit(0);
}

void* thread_func(void* id) {
	int idx = *((int*) id);
	
	for (;;) {
		send(socks[idx], buf, 512, 0);
	}

	pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
	
	struct sockaddr_in sink_id, cmd_id;

	bzero(&sink_id, sizeof(sink_id));
	sink_id.sin_family = PF_INET;
	sink_id.sin_addr.s_addr = inet_addr("127.0.0.1");
	sink_id.sin_port = htons(9999);

	bzero(&cmd_id, sizeof(cmd_id));
	cmd_id.sin_family = PF_INET;
	cmd_id.sin_addr.s_addr = inet_addr("127.0.0.1");
	cmd_id.sin_port = htons(9998);

	socks[0] = socket(AF_INET, SOCK_STREAM, 0);
	connect(socks[0], (struct sockaddr *) &cmd_id, sizeof(cmd_id));

	int now_clients = 12;
	for (int i = 1; i <= now_clients; i++) {
		socks[i] = socket(AF_INET, SOCK_STREAM, 0);
		connect(socks[i], (struct sockaddr *) &sink_id, sizeof(sink_id));
	}

	signal(SIGINT,  handler);
	signal(SIGTERM, handler);

	char message[] = { "/reset\n" };
	send(socks[0], message, 7, 0);
	int read_len = read(socks[0], buf, 100);


	pthread_t* thread_handles = (pthread_t*) malloc((now_clients + 1) * sizeof(pthread_t));

	for (int j = 1; j <= now_clients; j++) {
		pthread_create(&thread_handles[j], (pthread_attr_t*) NULL, thread_func, (void*) &j);
	}

	for (int i = 1; i <= now_clients; i++) {
		pthread_join(thread_handles[i], NULL);
	}
	

	return 0;
}
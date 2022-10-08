#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/signal.h>
using namespace std;

static struct timeval _t0;
static unsigned long long bytesent = 0; // alread sent

double tv2s(struct timeval *ptv) {
	return 1.0*(ptv->tv_sec) + 0.000001*(ptv->tv_usec);
}

void handler(int s) {
	struct timeval _t1;
	double t0, t1;
	gettimeofday(&_t1, NULL);
	t0 = tv2s(&_t0);
	t1 = tv2s(&_t1);
	fprintf(stderr, "\n%lu.%06lu %llu bytes sent in %.6fs (%.6f Mbps; %.6f MBps)\n",
		_t1.tv_sec, _t1.tv_usec, bytesent, t1-t0, 8.0*(bytesent/1000000.0)/(t1-t0), (bytesent/1000000.0)/(t1-t0));
	exit(0);
}

char buf[1001];

int main(int argc, char *argv[]) {
	if (argc < 2) {
		cout << "./tcpcbr speed(float)\n";
		exit(1);
	}
	double speed = stod(argv[1]);

	int sock_fd = socket(AF_INET, SOCK_RDM, 0);
	if (sock_fd < 0) {
        cout << "Error on socket init!\n";
        exit(1);
    }

	struct sockaddr_in server_id;

	bzero(&server_id, sizeof(server_id));
	server_id.sin_family = PF_INET;
	server_id.sin_addr.s_addr = inet_addr("140.113.213.213");
	server_id.sin_port = htons(10003);

	int err = connect(sock_fd, (struct sockaddr *) &server_id, sizeof(server_id));
	if (err < 0) {
		cout << "Error on connect to server!\n";
        exit(1);
	}

	
	memset(buf, 0, sizeof(buf));
	recv(sock_fd, buf, sizeof(buf), 0);
	cout << buf << '\n';
	

	cout << "Now on speed = " << speed << " MBps\n";
	
	signal(SIGINT,  handler);
	signal(SIGTERM, handler);

	gettimeofday(&_t0, NULL);


	int now_size = 30 * speed;
	while(1) {
		struct timespec t = { 0, 1 };

		int send_size = send(sock_fd, buf, now_size, MSG_OOB);
		bytesent += send_size;

		nanosleep(&t, NULL);
	}
	return 0;
}

// tcp socket no ack


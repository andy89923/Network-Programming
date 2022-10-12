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

#define MAX9 1000000000
#define MAX6 1000000

static struct timeval _t0, now_time;
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

char buf[10000000];

int main(int argc, char *argv[]) {
	if (argc < 2) {
		cout << "./tcpcbr speed(float)\n";
		exit(1);
	}
	double speed = stod(argv[1]);

	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0) {
        cout << "Error on socket init!\n";
        exit(1);
    }
    
    // Socket options
    int window_size = 100000;
    setsockopt(sock_fd, SOL_SOCKET, SO_SNDBUF, (char*) &window_size, sizeof(window_size));

    // int timeout = 0;
    // setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));

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

	int tol_size = speed * MAX6 * 0.9;
	int ave_size = tol_size / MAX6 - 10;
	int now_size = ave_size;

	int pre_size = 0;
	int mir_gap, sec_gap;

	while(1) {
		struct timespec t = { 0, 1 };
		
		gettimeofday(&now_time, NULL);


		mir_gap = now_time.tv_usec - _t0.tv_usec;
		sec_gap = now_time.tv_sec - _t0.tv_sec;

		pre_size = sec_gap * tol_size;

		if (now_time.tv_usec < _t0.tv_usec) mir_gap += MAX6;

		now_size = ave_size * mir_gap - (bytesent - pre_size) + ave_size;
		

		int send_size = send(sock_fd, buf, now_size, 0);

		bytesent += send_size;
		

		nanosleep(&t, NULL);
	}
	return 0;
}
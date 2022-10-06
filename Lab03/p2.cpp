#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
using namespace std;

#define MB 1000000

int main(int argc, char const *argv[]) {
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0) {
        cout << "Error on socket init!\n";
        exit(1);
    }

	struct sockaddr_in server_id;

	bzero(&server_id, sizeof(server_id));
	server_id.sin_family = PF_INET;
	server_id.sin_addr.s_addr = inet_addr("140.113.213.213");
	server_id.sin_port = htons(10003);

	if (argc < 2) {
		cout << "./tcpcbr speed(float)\n";
		exit(0);
	}  
	float speed = stod(argv[1]);

	cout << "Now on speed = " << speed << " MBps\n";
	



	return 0;
}
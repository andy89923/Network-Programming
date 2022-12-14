
#define MAX_SED   1160
#define MAX_PKT   1200
#define MAX_BUF   1210
#define SEG         40
#define NUM_FILE  1005
#define TIMEOUT      5
#define ACK_LEN      5
#define RECV_SIZ 30000

struct File {

	string name;
	bool send[SEG];

	int leng[SEG];
	int max_indx, cnt_indx; 

	char data[SEG][MAX_PKT];

	void clear() {
		for (int i = 0; i < SEG; i++) {
			send[i] = false;
			leng[i] = 0;
			memset(data[i], 0, MAX_PKT);
		}
		max_indx = 0;
		cnt_indx = 0;
	}
};

#include <arpa/inet.h>

void udp_socket(int& sock, sockaddr_in& server_id, int port) {
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (sock < 0) {
        cout << "Error on socket init!\n";
        exit(1);
    }    
    const int opt = true;
    socklen_t optlen = sizeof(opt);
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, optlen);
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, optlen);

	int recv_siz = RECV_SIZ;
	setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &recv_siz, sizeof(recv_siz));
    
    bzero(&server_id, sizeof(server_id));
	server_id.sin_family = AF_INET;
	server_id.sin_port = htons(port);

	int r = bind(sock, (sockaddr*) &server_id, sizeof(sockaddr_in));
	if (r < 0) {
		cout << "Error on bind!\n";
		exit(1);
	}
}

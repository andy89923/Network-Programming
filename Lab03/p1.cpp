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
char tar[] = { "==== END DATA ====" };

int main() {
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0) {
        cout << "Error on socket init!\n";
        exit(1);
    }

	struct sockaddr_in server_id;

	bzero(&server_id, sizeof(server_id));
	server_id.sin_family = PF_INET;
	server_id.sin_addr.s_addr = inet_addr("140.113.213.213");
	server_id.sin_port = htons(10002);

	int err = connect(sock_fd, (struct sockaddr *) &server_id, sizeof(server_id));
	if (err < 0) {
		cout << "Error on connect to server!\n";
        exit(1);
	}

	recv(sock_fd, buf, sizeof(buf), 0);
	cout << buf << '\n';

	char message[] = { "GO\n" };
	send(sock_fd, message, 3, 0);

	memset(buf, 0, sizeof(buf));

	int total_size = 0;
	int now_size = recv(sock_fd, buf, sizeof(buf) - 1, 0);

	while (true) {
		memset(buf, 0, sizeof(buf));

		int now_size = recv(sock_fd, buf, 1, 0);
		if (now_size < 0){
			cout << "Error on recv!\n";
			exit(0);
		}
		total_size += now_size;
		// cout << total_size << '\n';

		int ok = 0;
		for (int i = 0; i < now_size; i++) if (buf[i] == tar[0]) {
			ok = 1;
			cout << "Break\n";
			break;

			int cnt = 0;
			for (int j = 0; j < sizeof(tar) - 1; j++) {
				if (buf[i + j] != tar[j]) break;
				cnt += 1;
			}
			if (cnt == sizeof(tar) - 1) {
				ok = 1;
				cout << "End of the data!\n";
				break;
			}
		}
		
		if (ok) break;
	}
	total_size -= 2;
	cout << "Totle size = " << total_size << '\n';

	memset(buf, 0, sizeof(buf));

	recv(sock_fd, buf, 1000, 0);
	cout << buf << '\n';

	string s = std::to_string(total_size) + "\n";
	char const *pchar = s.c_str(); 

	cout << "Sending answer... : " << pchar << '\n';
	send(sock_fd, pchar, sizeof(pchar), 0);

	sleep(1);

	cout << "Answer: \n\n";

	memset(buf, 0, sizeof(buf));

	recv(sock_fd, buf, 1000, 0);
	cout << buf << '\n';

	return 0;
}

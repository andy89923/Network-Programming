#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
using namespace std;

#define BUFSIZE 10000
#define MAXCONN 1001

char buf[BUFSIZE];

struct User {
	int fd_num;
	int port;

	in_addr ip;
	string name;
};

User clients[MAXCONN];
int num_clients;
int max_fd, max_po;
fd_set rcv_set, all_set;

string get_time_str() {
	string tim_s;

	time_t t = time(0);
	tm *now = localtime(&t);

	tim_s += to_string(1900 + now -> tm_year) + "-";
	tim_s += to_string(   1 + now -> tm_mon)  + "-";
	tim_s += to_string(   1 + now -> tm_mday) + " ";
	tim_s += to_string(now -> tm_hour) + ":";
	tim_s += to_string(now -> tm_min)  + ":";
	tim_s += to_string(now -> tm_sec);

	return tim_s;
}

void join_server_message(string new_user_name) {
	// 2022-10-05 21:50:08 *** User <fabulous caudata> has just landed on the server
	string ss = "\n";
	string tim_s = get_time_str();

	ss += tim_s;
	ss += " *** User <" + new_user_name + "> has just landed on the server\n";

	char const *pchar = ss.c_str(); 
	for (int i = 0; i <= max_po; i++) if (clients[i].fd_num != -1) {
		send(clients[i].fd_num, pchar, ss.length(), 0);
	}
}

void welcome_message(int user_poi) {
	// 2022-10-05 15:45:00 *** Welcome to the simple CHAT server
	// 2022-10-05 15:45:00 *** Total 2 users online now. Your name is <becoming babirusa>
	
	string ss = "", tim_s = get_time_str();

	ss += tim_s + " *** Welcome to the simple CHAT server\n";
	ss += tim_s + " *** Total " + to_string(num_clients) + " users online now. Your name is ";
	ss += "<" + clients[user_poi].name + ">\n";

	// send
	char const *pchar = ss.c_str(); 
	send(clients[user_poi].fd_num, pchar, ss.length(), 0);

	join_server_message(clients[user_poi].name);
}

void list_all_user(int sock, int user_poi) {
	string ss = "", now_row;

	ss += "----------------------------------\n";

	for (int i = 0; i <= max_po; i++) if (clients[i].fd_num != -1) {
		now_row = "";
		if (user_poi == i) 
			now_row += " * ";
		else
			now_row += "   ";

		now_row += "<" + clients[i].name + "> ";

		while (now_row.length() < 20) now_row += " ";

		string ips = inet_ntoa(clients[i].ip);
		now_row += ips + ":";
		now_row += to_string(clients[i].port);
		now_row += "\n";

		ss += now_row;
	} 
	ss += "----------------------------------\n";

	char const *pchar = ss.c_str(); 
	send(sock, pchar, ss.length(), 0);

	cout << ss << '\n';
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
	r = listen(sock, MAXCONN);
}

void select_init(int &socket) {
	FD_ZERO(&all_set);
	FD_ZERO(&rcv_set);

	num_clients = 0;
	max_fd = socket;
	max_po = -1;

	for (int i = 0; i < MAXCONN; i++) {
		clients[i].fd_num = -1;
	}

	FD_SET(socket, &all_set);
}


/*
(to the user)
	2022-10-05 16:05:34 *** Nickname changed to <hello, world!>
(to the rest of the online users)
	2022-10-05 16:06:03 *** User <jubilant cob> renamed to <hello, world!>
*/
void renamed_notification(string old_name, string new_name) {
	string ss = "\n";
	string tim_s = get_time_str();

	ss += tim_s;
	ss += " *** User <" + old_name + "> renamed to ";
	ss += "<" + new_name + ">\n";

	char const *pchar = ss.c_str(); 
	for (int i = 0; i <= max_po; i++) if (clients[i].fd_num != -1) {
		if (clients[i].name != new_name)
			send(clients[i].fd_num, pchar, ss.length(), 0);
	}
}

void change_user_name(int user_poi, int read_len) {
	string old_name = clients[user_poi].name;

	clients[user_poi].name = "";
	for (int j = 6; j < read_len - 1; j++) clients[user_poi].name += buf[j];

	string ss = "";
	string tim_s = get_time_str();

	ss += tim_s;
	ss += " ***  Nickname changed to <";
	ss += clients[user_poi].name + ">\n";
 
	// send
	char const *pchar = ss.c_str(); 
	send(clients[user_poi].fd_num, pchar, ss.length(), 0);

	renamed_notification(old_name, clients[user_poi].name);
}

void left_server_message(string name) {
	// 2022-10-05 16:23:05 *** User <flayed jabiru> has left the server

	string ss = "\n";
	string tim_s = get_time_str();

	ss += tim_s;
	ss += " *** User <" + name + "> has left the server\n";

	char const *pchar = ss.c_str(); 
	for (int i = 0; i <= max_po; i++) if (clients[i].fd_num != -1) {
		send(clients[i].fd_num, pchar, ss.length(), 0);
	}
}

int main(int argc, char* argv[]) {
	srand(time(NULL));

	cout << "FD_SETSIZE = " << FD_SETSIZE << '\n';

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

	signal(SIGPIPE, SIG_IGN);

	for (;;) {

		rcv_set = all_set;

		int num_ready = select(max_fd + 1, &rcv_set, NULL, NULL, NULL);
		if (FD_ISSET(sock, &rcv_set)) {
			connect_fd = accept(sock, (sockaddr*) &client_id, (socklen_t*) &info_len);

			for (int i = 0; i < FD_SETSIZE; i++) {
				if (clients[i].fd_num < 0) {
					clients[i].fd_num = connect_fd;
					max_po = max(max_po, i);

					clients[i].ip = client_id.sin_addr;
					clients[i].port = ntohs(client_id.sin_port); 
					clients[i].name = to_string(rand());
					num_clients += 1;

					// Log message
					// * client connected from 140.113.1.1:38864
					cout << "client connected from " << inet_ntoa(clients[i].ip);
					cout << ":" << to_string(clients[i].port) << '\n';

					welcome_message(i);
					break;
				}
				if (i == FD_SETSIZE - 1) {
					perror("Too many clients\n");
					exit(1);
				}
			}

			FD_SET(connect_fd, &all_set);
			max_fd = max(max_fd, connect_fd);

			num_ready -= 1;
			if (num_ready <= 0) continue;
		}

		int now_sock, disconnect;
		for (int i = 0; i <= max_po; i++) {
			if ((now_sock = clients[i].fd_num) < 0) continue;
			
			disconnect = 0;
			if (FD_ISSET(now_sock, &rcv_set)) {

				int read_len = read(now_sock, buf, BUFSIZE);
				if (read_len == 0) {
					// connection closed by client
					close(now_sock);
					FD_CLR(now_sock, &all_set);
					
					clients[i].fd_num = -1;
					num_clients -= 1;

					// Disconnect message
					// * client 140.113.1.1:38864 disconnected
					cout << "client " << inet_ntoa(clients[i].ip);
					cout << ":" << to_string(clients[i].port) << " disconnected\n";
					
					left_server_message(clients[i].name);

					disconnect = 1;
				}

				if (!disconnect) if (buf[0] == '/') {
					// command 

					if (buf[1] == 'w' && buf[2] == 'h' && buf[3] == 'o') {
						list_all_user(now_sock, i);
					}
					
					else if (buf[1] == 'n' && buf[2] == 'a' && buf[3] == 'm' && buf[4] == 'e') {
						change_user_name(i, read_len);
					}
					else {
						// 2022-10-05 16:13:40 *** Unknown or incomplete command </name>
						string ss = get_time_str();
						string cd = buf;

						ss += " *** Unknown or incomplete command <" + cd + ">\n";

						char const *pchar = ss.c_str(); 
						send(clients[i].fd_num, pchar, ss.length(), 0);
					}
				} else {
					// text
					// 2022-10-05 16:20:12 <flayed jabiru> hey guys, how's everything?
					string ss = get_time_str();
					ss += " <" + clients[i].name + "> ";
					for (int j = 0; j < read_len; j++) ss += buf[j];

					char const *pchar = ss.c_str(); 
					for (int k = 0; k <= max_po; k++) if (clients[k].fd_num != -1) {
						if (k == i) continue;
						send(clients[k].fd_num, pchar, ss.length(), 0);
					}
				}

				memset(buf, 0, sizeof(buf));

				num_ready -= 1;
				if (num_ready <= 0) break;
			}
		}
	}
	return 0;
}
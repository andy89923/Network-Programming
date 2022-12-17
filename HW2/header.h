#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
using namespace std;

#define MAXZONE    10
#define MAXBUF   2000
#define DNS_PORT   53


string type_to_str[17] = { "", 
	"A",    "NS", "MD", "MF", "CNAME",    //  1 -  5
	"SOA",  "MB", "MG", "MR", "NULL",     //  6 - 10
	"WKS", "PTR", "HINFO", "MINFO", "MX", // 11 - 15
	"TXT" 
};

struct Record {

	string name;

	int ttl, type;
};

struct Zone {

	string name;
	vector<Record> v;

	void init() {
		v.clear();

		
	}

	void setup(string name, string path) {
		this -> init();
		this -> name = name;




	}
};


int zone_cnt = 0;

void init_message() {
	string now = "\n";
	now += " ░░░▐▀▀▄█▀▀▀▀▀▒▄▒▀▌░░░░\n";
	now += " ░░░▐▒█▀▒▒▒▒▒▒▒▒▀█░░░░░\n";
	now += " ░░░░█▒▒▒▒▒▒▒▒▒▒▒▀▌░░░░\n";
	now += " ░░░░▌▒██▒▒▒▒██▒▒▒▐░░░░\n";
	now += " ░░░░▌▒▒▄▒██▒▄▄▒▒▒▐░░░░\n";
	now += " ░░░▐▒▒▒▀▄█▀█▄▀▒▒▒▒█▄░░\n";
	now += " ░░░▀█▄▒▒▐▐▄▌▌▒▒▄▐▄▐░░░\n";
	now += " ░░▄▀▒▒▄▒▒▀▀▀▒▒▒▒▀▒▀▄░░\n";
	now += " ░░█▒▀█▀▌▒▒▒▒▒▄▄▄▐▒▒▐░░\n";
	now += " ░░░▀▄▄▌▌▒▒▒▒▐▒▒▒▀▒▒▐░░\n";
	now += " ░░░░░░░▐▌▒▒▒▒▀▄▄▄▄▄▀░░\n";
	now += " ░░░░░░░░▐▄▒▒▒▒▒▒▒▒▐░░░\n";
	now += " ░░░░░░░░▌▒▒▒▒▄▄▒▒▒▐░░░\n";

	now += "\n";

	cout << now;
}

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

    bzero(&server_id, sizeof(server_id));
	server_id.sin_family = AF_INET;
	server_id.sin_port = htons(port);

	int r = bind(sock, (sockaddr*) &server_id, sizeof(sockaddr_in));
	if (r < 0) {
		cout << "Error on bind!\n";
		exit(1);
	}
}
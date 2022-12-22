#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
using namespace std;

#define MAXZONE    10
#define MAXBUF   2000
#define DNS_PORT   53


string typ_to_str[17] = { "", 
	"A",    "NS", "MD", "MF", "CNAME",    //  1 -  5
	"SOA",  "MB", "MG", "MR", "NULL",     //  6 - 10
	"WKS", "PTR", "HINFO", "MINFO", "MX", // 11 - 15
	"TXT" 
};

int str_to_typ(string now) {
	for (int i = 1; i <= 16; i++) {
		if (typ_to_str[i] == now) return i;
	}
	return -1;
}
/*
<domain>
<NAME>,<TTL>,<CLASS>,<TYPE>,<RDATA>
<NAME>,<TTL>,<CLASS>,<TYPE>,<RDATA>

<RDATA>
A     : <ADDRESS>
AAAA  : <ADDRESS>
NS    : <NSDNAME>
CNAME : <CNAME>
SOA   : <MNAME> <RNAME> <SERIAL> <REFRESH> <RETRY> <EXPIRE> <MINIMUM>
MX    : <PREFERENCE> <EXCHANGE>
TXT   : <TXT-DATA>

@,   3600, IN, MX, 10 mail.example1.org.
dns, 3600, IN, A,  140.113.123.1
www, 300 , IN, A,  140.113.123.80
*/

struct Record {

	string name, clss;
	int ttl, typ;
	vector<string> data;

	void clear() {
		data.clear();
	}
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

		ifstream file;
		file.open(path);

		char now[2000] = {};
		char delimeter[] = ",";

		file >> now; // zone name
		file.getline(now, 200);

		int ttl, typn;

		while (file.getline(now, 200)) {
			Record r; r.clear();

			char* tmp = strtok(now, delimeter);

			r.name = tmp;

			tmp = strtok(NULL, delimeter);
			r.ttl = atoi(tmp);

			tmp = strtok(NULL, delimeter);
			r.clss = tmp;

			tmp = strtok(NULL, delimeter);
			r.typ = str_to_typ(tmp);

			// cout << r.name << ' ' << r.ttl << ' ' << r.clss << ' ' << r.typ << "\n  data = ";

			tmp = strtok(NULL, "\n");
			while (tmp) {
				r.data.push_back(tmp);
				tmp = strtok(NULL, "\n");
			}

			// for (auto i : r.data)
			// 	cout << i << ' ';
			// cout << '\n';

			v.push_back(r);
		}
		file.close();
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
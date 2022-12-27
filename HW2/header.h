#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/udp.h>  // Provides declarations for udp header
#include <iostream>
using namespace std;

#define MAXZONE    10
#define MAXBUF   2000
#define DNS_PORT   53


string typ_to_str[30] = { "", 
	"A",    "NS", "MD", "MF", "CNAME",    //  1 -  5
	"SOA",  "MB", "MG", "MR", "NULL",     //  6 - 10
	"WKS", "PTR", "HINFO", "MINFO", "MX", // 11 - 15
	"TXT" 
};

int str_to_typ(string now) {
	for (int i = 1; i <= 16; i++) {
		if (typ_to_str[i] == now) return i;
	}
	return 28; // AAAA
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

	string name, clss, sufx;

	int ttl, typ;
	vector<string> data;

	char b[100];
	int len;

	void clear() {
		data.clear();
		len = -1;
		memset(b, 0, sizeof(b));
	}

	void fix_dot(char* c, int len) {
		for (int i = 0; i < len; i++) {
			int poi = i, t = 0;

			while (i + 1 < len && c[i + 1] != 0 && c[i + 1] != '.') {
				t += 1;
				i += 1;
			}
			c[poi] = t;
		}
	}

	void construct_basic() {
		if (name.length() != 0) {
			b[len++] = 0;
			memcpy(b + len, name.c_str(), name.length());
			len += name.length();
		}
		b[len++] = 0;
		memcpy(b + len, sufx.c_str(), sufx.length());
		len += sufx.length() - 1;

		fix_dot(b, len);

		b[len++] = 0;

		unsigned short type = htons(typ);
		memcpy(b + len, &type, sizeof(type));
		len += sizeof(type);

		unsigned short clas = htons(1);
		memcpy(b + len, &clas, sizeof(clas));
		len += sizeof(clas);

		int t_ttl = htonl(ttl);
		memcpy(b + len, &t_ttl, sizeof(ttl));
		len += sizeof(ttl);
	}

	void construct() {
		len = 0;
		if (typ_to_str[typ] == "A" or typ == 28) {
			construct_basic();			

			unsigned short d_leng = htons(4);
			memcpy(b + len, &d_leng, sizeof(d_leng));
			len += sizeof(d_leng);

			if (typ == 1) {
				unsigned int ip = inet_addr(data[0].c_str());
				memcpy(b + len, &ip, sizeof(ip));
				len += sizeof(ip);
			}
			if (typ == 28) {
				unsigned long long ip;
				inet_pton(AF_INET6, data[0].c_str(), &ip);
				memcpy(b + len, &ip, sizeof(ip));
				len += sizeof(ip);
			}
		}
		if (typ_to_str[typ] == "NS") {
			construct_basic();

			unsigned short d_leng = 0, tmp = len;
			len += sizeof(d_leng);

			b[len++] = 0;
			memcpy(b + len, data[0].c_str(), data[0].length());
			fix_dot(b + len - 1, data[0].length() - 1);

			len += data[0].length() - 1;
			b[len - 1] = 0;

			d_leng = data[0].length();
			d_leng = htons(d_leng);
			memcpy(b + tmp, &d_leng, sizeof(d_leng));
		}
		if (typ_to_str[typ] == "MX") {
			construct_basic();

			unsigned short d_leng = 0, tmp = len;
			len += sizeof(d_leng);

			unsigned short pre = stoi(data[0]);
			pre = htons(pre);
			memcpy(b + len, &pre, sizeof(pre));
			len += sizeof(pre);


			b[len++] = 0;
			memcpy(b + len, data[1].c_str(), data[1].length());
			fix_dot(b + len - 1, data[1].length() - 1);

			len += data[1].length() - 1;
			b[len - 1] = 0;

			d_leng = data[1].length() + 2;
			d_leng = htons(d_leng);
			memcpy(b + tmp, &d_leng, sizeof(d_leng));
		}
	}

	int push(char* packet) {
		if (len <= 0) construct();

		// removed after developed
		if (len <= 0) return 0;

		memcpy(packet, b, len);

		return len;
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

		char now[201] = {};
		char delimeter[] = ", \r\n";

		file >> now; // zone name
		file.getline(now, 200);

		int ttl, typn;

		while (file.getline(now, 200)) {
			Record r; r.clear();

			char* tmp = strtok(now, delimeter);
			r.name = tmp;

			if (r.name == "@") r.name = "";

			tmp = strtok(NULL, delimeter);
			r.ttl = atoi(tmp);

			tmp = strtok(NULL, delimeter);
			r.clss = tmp;

			tmp = strtok(NULL, delimeter);
			r.typ = str_to_typ(tmp);

			tmp = strtok(NULL, delimeter); 
			while (tmp != NULL) {
				r.data.push_back(tmp);
				tmp = strtok(NULL, delimeter);
			}

			// cout << "Start\n";
			// cout << r.name << ' ' << r.ttl << ' ' << r.clss << ' ' << r.typ << '\n';
			// for (auto s : r.data) cout << s << '-';
			// cout << '\n';

			r.sufx = this -> name;
			v.push_back(r);
		}
		file.close();
	}
};


int zone_cnt = 0;

void init_message() {
	string now = "\n";
	now += "==========================\n";
	now += "===== HW2 DNS Server =====\n";
	now += "==========================\n";
	now += "                        \n";
	now += "  ░░░▐▀▀▄█▀▀▀▀▀▒▄▒▀▌░░░░\n";
	now += "  ░░░▐▒█▀▒▒▒▒▒▒▒▒▀█░░░░░\n";
	now += "  ░░░░█▒▒▒▒▒▒▒▒▒▒▒▀▌░░░░\n";
	now += "  ░░░░▌▒██▒▒▒▒██▒▒▒▐░░░░\n";
	now += "  ░░░░▌▒▒▄▒██▒▄▄▒▒▒▐░░░░\n";
	now += "  ░░░▐▒▒▒▀▄█▀█▄▀▒▒▒▒█▄░░\n";
	now += "  ░░░▀█▄▒▒▐▐▄▌▌▒▒▄▐▄▐░░░\n";
	now += "  ░░▄▀▒▒▄▒▒▀▀▀▒▒▒▒▀▒▀▄░░\n";
	now += "  ░░█▒▀█▀▌▒▒▒▒▒▄▄▄▐▒▒▐░░\n";
	now += "  ░░░▀▄▄▌▌▒▒▒▒▐▒▒▒▀▒▒▐░░\n";
	now += "  ░░░░░░░▐▌▒▒▒▒▀▄▄▄▄▄▀░░\n";
	now += "  ░░░░░░░░▐▄▒▒▒▒▒▒▒▒▐░░░\n";
	now += "  ░░░░░░░░▌▒▒▒▒▄▄▒▒▒▐░░░\n";
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
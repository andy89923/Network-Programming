#include <bits/stdc++.h>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
using namespace std;
#define agole a[go][le]
#define pb push_back
#define ppb pop_back
#define MAX 5010

static const char* socket_path = "/sudoku.sock";
char buf[MAX];

bool ok;
int  a[11][11];
char in[90];
set<int> s[11], f[11], c[11];

struct point{
    int x;
    int y;
};

vector<point> q;

int cell(int l, int k) {
    int r = 0, o, p;
    o = (l - 1) / 3 + 1;
    p = (k - 1) / 3 ;
    r = o + p * 3;
    return r;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

void init() {
    memset(a, 0, sizeof(a))	;
    while(!q.empty()) q.pop_back();
    for(int i = 1; i <= 9; i++) {
        for(int k = 1; k <= 9; k++) {
            if(s[k].count(i)) s[k].erase(i);
            if(f[k].count(i)) f[k].erase(i);
            if(c[k].count(i)) c[k].erase(i);
        }
    }
    ok = true;
}

void build() {
    int le = 1, go = 1, z;
    for(int i = 0; i < 81; i ++) {
        z = in[i] - '0';
        if(in[i] == '.') {
            point now;
            now.x = go, now.y = le;
            q.pb(now);
        }
        else {
            int gg;
            gg = cell(go, le);
            if( f[go].count(z) || s[le].count(z) || c[gg].count(z) ) {
                ok = false;
                return;
            }
            f[go].insert(z), s[le].insert(z), c[gg].insert(z);
            agole = z;
        }
        go ++;
        if(go > 9) go = 1, le ++;
    }
}

void pri(int sock) {
	int cnt = 0;
	for (int i = 1; i <= 9; i++) {
		for (int k = 1; k <= 9; k++) {
			if (in[cnt] == '.') {
				cout << a[k][i];

				string now = "V " + to_string(i - 1) + " " + to_string(k - 1) + " " + to_string(a[k][i]);
				memset(buf, 0, sizeof(buf));
				memcpy(buf, now.c_str(), now.length());

				send(sock, buf, now.length() + 1, 0);
				
				memset(buf, 0, sizeof(buf));
				int data_len = recv(sock, buf, MAX, 0);
			}
			else
				cout << ".";

			cnt += 1;
		}
	}
	cout << '\n';
}

bool dfs(point fa, point now) {
    int x = now.x, y = now.y, gg;
    point next;
    // coding ~~~~~~~~~~~~~~~~
    gg = cell(x, y);
    for(int i = 1; i <= 9; i++) {
        if(f[x].count(i) || s[y].count(i) || c[gg].count(i)) continue;
        a[x][y] = i;
        f[x].insert(i), s[y].insert(i), c[gg].insert(i);
        
        if( q.empty() ) return true;
        next = q.back();
        q.ppb();
        
        if( dfs(now, next) )  return true;
        else {
            q.pb(next);
            f[x].erase(i), s[y].erase(i), c[gg].erase(i);
            a[x][y] = 0;
            continue;
        }
    }
    return false;
}

void solve(int sock) {
    point now, fa;
    fa.x = fa.y = 0;
    now = q.back();
    q.ppb();
    
    if( dfs(fa, now) ) pri(sock);
}

int main() {

	int sock = sock = socket(AF_UNIX, SOCK_STREAM, 0);
	struct sockaddr_un remote;

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, socket_path);

	int data_len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	
	if (connect(sock, (struct sockaddr*) &remote, data_len) == -1) {
		cerr << "Error on connect\n";
		exit(1);
	}
	cout << "Connect success!\n";

	memset(buf, 0, sizeof(buf));

	string now = "S";
	memcpy(buf, now.c_str(), 1);

	send(sock, buf, 2, 0);

	data_len = recv(sock, buf, MAX, 0);

	for (int i = 4; i < 4 + 81; i++) {
		in[i - 4] = buf[i];
	}
	cout << buf << '\n';
	cout << "Board: " << in << '\n';

	build();
	solve(sock);

	memset(buf, 0, sizeof(buf));

	// data_len = recv(sock, buf, MAX, 0);

	// cout << buf << '\n';


	memset(buf, 0, sizeof(buf));	
	
	now = "C";
	memcpy(buf, now.c_str(), 1);
	send(sock, buf, 2, 0);

	memset(buf, 0, sizeof(buf));
	data_len = recv(sock, buf, MAX, 0);

	cout << buf << '\n';

	return 0;
}

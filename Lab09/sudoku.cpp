/*input
24.13..8.1382..4.565..8.12.38.6.2.97526..7..17...58.6446.8.1952.125.374.9.5.2631.
e
*/
#include <bits/stdc++.h>
using namespace std;
#define agole a[go][le]
#define pb push_back
#define ppb pop_back
#define IOS cin.tie(0), cout.sync_with_stdio(false);

bool ok;
int  a[11][11];                  // [ x ] [ y ]
char in[90];
set<int> s[11], f[11], c[11];    // s - le - y  // f - go - x  // 9 cell

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

void pri() {
    for(int i = 1; i <= 9; i++)
        for(int k = 1; k <= 9; k++)
            printf("%d",a[k][i]);
    puts("");
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

void solve() {
    point now, fa;
    fa.x = fa.y = 0;
    now = q.back();
    q.ppb();
    
    if( dfs(fa, now) ) pri();
    else puts("No solution.");
}

int main() {
    IOS;
    while(cin >> in) {
        init();
        if(in[0] == 'e') return 0;
        build();
        
        if(!ok) puts("No solution.");
        else solve();
    }
    return 0;
}
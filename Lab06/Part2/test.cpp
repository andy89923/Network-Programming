#include <iostream>
#include <sys/time.h>
#include <cstdlib>
#include <unistd.h>
using namespace std;

int main() {
	
	struct timeval start;

	gettimeofday(&start, NULL);

	cout << start.tv_sec << ' ' << start.tv_usec << '\n';

	sleep(1);

	gettimeofday(&start, NULL);
	cout << start.tv_sec << ' ' << start.tv_usec << '\n';
}
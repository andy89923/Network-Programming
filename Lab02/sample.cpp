#include <iostream>
#include <fcntl.h>
#include <unistd.h>
using namespace std;

int main() {

	int file = open("test.txt", ios::in);

	char c[100];
	cout << read(file, c, 1) << '\n';
	cout << c[0] << '\n';

	cout << read(file, c, 1) << '\n';
	cout << c[0] << '\n';


	/* 
	lseek(int fd, off_t offset, int whence);
	SEEK_SET 從距檔案開頭offset 位移量為新的讀寫位置. SEEK_CUR 以目前的讀寫位置往後增加offset 個位移量.
	SEEK_END 將讀寫位置指向檔案尾後再增加offset 個位移量. 
	
	當whence 值為SEEK_CUR 或SEEK_END 時, 引數offset 允許負值的出現.
	*/
	cout << lseek(file, 0, SEEK_SET) << '\n';

	cout << read(file, c, 1) << '\n';
	cout << c[0] << '\n';

	return 0;
}
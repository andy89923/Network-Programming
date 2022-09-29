#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <arpa/inet.h>
#include <fstream>
using namespace std;

#define MAX 3368600

unsigned char c[MAX];

struct header_s {

	char magic[4];	
	int32_t string_off;
	int32_t content_off;
	int32_t num_files;

}__attribute__((packed)) header;

struct file_e {
	
	int32_t name_off;
	int32_t file_siz;   // input is Big-Endian
	int32_t cont_off;
	uint64_t check_sum;  // input is Big-Endian

}__attribute__((packed));

file_e filees[MAX];
string file_names[MAX];

int main(int argc, char const *argv[]) {

	const char* pako_file_name = argv[1];
	const char* targ_path = argv[2];

	// char* pako_file_name = "example.pak";

	cout << "Now file = " << pako_file_name << '\n';

	int file;
	if (pako_file_name == "")
		file = open("example.pak", ios::in);
	else
		file = open(pako_file_name, ios::in);

	if (file < 0) {
		cout << "Error!\n";
		return 0;
	}

	int file_poi = 0;

	char *header_p = (char*) &header;
	read(file, header_p, sizeof(header_s));
	file_poi += sizeof(header_s);

	cout << "Number of files: " << header.num_files << '\n';


	// File_E
	for (int i = 0; i < header.num_files; i++) {
		char* buf = (char*) &filees[i];

		read(file, buf, sizeof(file_e));
		file_poi += sizeof(file_e);

		filees[i].file_siz = ntohl(filees[i].file_siz);

		filees[i].check_sum = __builtin_bswap64(filees[i].check_sum);
	}

	// file names
	lseek(file, header.string_off, SEEK_SET);
	for (int i = 0; i < header.num_files; i++) {
		char c[1];

		string now = "";
		read(file, c, 1);

		while (c[0] != 0x00) {
			now += c;
			read(file, c, 1);
			file_poi += 1;
		}
		cout << "File " << i + 1 << ": " << now << ", Size(byte) = " << filees[i].file_siz<< '\n';

		file_names[i] = now;
	}
	cout << "--------------------------\n";


	for (int i = 0; i < header.num_files; i++) {
		uint64_t checksum = 0, now = 0;

		string context = "";
		lseek(file, header.content_off + filees[i].cont_off, SEEK_SET);

		read(file, c, filees[i].file_siz);

		for (int j = 0; j < filees[i].file_siz; j++) {
			now <<= 8;	
			now += (uint64_t) c[j];

			if (j != 0 && j % 8 == 7) {
				checksum = checksum ^ now;
				// cout << hex << _OSSwapInt64(now) << ' ';
				now = 0;
			}

			context += (char) c[j];
		}
		if (filees[i].file_siz % 8 != 0) {
			for (int j = 0; j < 8 - (filees[i].file_siz % 8); j++) {
				now <<= 8;	
			}
		}

		// cout << hex << _OSSwapInt64(now) << '\n';
		checksum = checksum ^ now;

		cout << hex << __builtin_bswap64(checksum) << '\n' << filees[i].check_sum << "\n\n";

		if (__builtin_bswap64(checksum) == filees[i].check_sum) {
			string now_file_name = targ_path + '/' + file_names[i];

			ofstream now_file(now_file_name);

			now_file << context;

			now_file.close();

			cout << file_names[i] << " -> pass checksum\n\n";
		}
	}

	return 0;
}
#include <sys/stat.h>
#include "utils.h"

using namespace std;

int
string_to_int(const string s) {
    int x;
    return from_string(s, x);
}

size_t
get_file_size(const string fn) {
    struct stat filestatus;
    stat(fn.c_str(), &filestatus);
    return filestatus.st_size;
}

byte *
read_file(const string fn) {
    size_t size = get_file_size(fn);
    byte *data = new byte[size];
    if (!data) {
        cerr << "could not allocate " << size << " bytes" endl;
        return 0;
    }
    ifstream f;
    f.open(filename, ios::in | ios::binary);
    if (!f.is_open()) {
	cerr << "could not open " << filename << endl;
        // Return empty map on error
	return map<string, vector<offset_t>>();
    }

    f.read((char *)data, size);
}
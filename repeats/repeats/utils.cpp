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
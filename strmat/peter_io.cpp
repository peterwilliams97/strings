#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "peter_io.h"

using namespace std;

// !@#$ Hack to limit memory usage.
static const long max_file_bytes = 999000;

FileData::FileData(long size, byte *data) {
    cout << "  FileData(" << size << ")" << endl;
    _size = size;
     _data = data;
}
FileData::~FileData() {
    delete[] _data;
}
int FileData::get_size() { return _size; }
byte *FileData::get_data() { return _data; }

FileData read_file_data(string file_name)
{
    ifstream file(file_name, ios::in|ios::binary|ios::ate);
    if (!file.is_open()) {
        cerr << "Error reading " << file_name.c_str() << endl;
        return FileData(-1, 0);
    }
    long file_size = (long)file.tellg();
    long size = min(max_file_bytes, file_size);
    cout << " max_file_bytes = " << max_file_bytes << ", file_size = " << file_size << " => size = " << size  << endl;
    byte *data = new byte[size];
    file.seekg(0, ios::beg);
    file.read((char *)data, size);
    file.close();
    return FileData(size, data);
}



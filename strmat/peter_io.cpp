#include <string>
#include <iostream>
#include <fstream>
#include "peter_io.h"

using namespace std;

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
    long size = (long)file.tellg();
    byte *data = new byte[size];
    file.seekg(0, ios::beg);
    file.read((char *)data, size);
    file.close();
    return FileData(size, data);
}


//static void test()
//{
//    string a = "a";
//    string b = "b";
//    string c = "c";
//    c = a +  b;
//    cout << "a=" << a << endl;
//    cout << "b=" << b << endl;
//    cout << "c=" << c << endl;
//}
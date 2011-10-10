#ifndef _PETER_IO_H_
#define _PETER_IO_H_

#include <iostream>
#include "peter_common.h"

class FileData {
    int _size;
    byte *_data;
public:
    FileData(long size, byte *data) {
        _size = size;
        _data = data;
    }
    FileData() { FileData(0, 0); }
    ~FileData() {
        delete[] _data;
    }
    int get_size() { return _size; }
    byte *get_data() { return _data; }
};

FileData read_file_data(std::string file_name);


#endif // #define _PETER_IO_H_
#ifndef _PETER_IO_H_
#define _PETER_IO_H_

#include <iostream>

typedef unsigned char byte;

struct FileData {
    int _size;
    byte *_data;
    FileData(long size, byte *data) {
        _size = size;
        _data = data;
    }
};

FileData read_file_data(std::string file_name);


#endif // #define _PETER_IO_H_
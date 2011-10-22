#ifndef _PETER_COMMON_H_
#define _PETER_COMMON_H_

#include <string>

// Peter's definitions

typedef int BOOL;
#define FALSE 0
#define TRUE 1
#define ERROR 0
#define OK 1

#ifdef PETER_GLOBAL
 #define ALPHABET_SIZE 0
#else
 #define ALPHABET_SIZE 256
#endif

typedef unsigned char byte;
typedef unsigned char CHAR_TYPE;

template <class T>
inline static const T *dup_data(int len, const T *data)
{
    T *dup = new T[len];
    return (T *)memcpy(dup, data, len*sizeof(T));
}

template <class T>
class GenString
{
    const int _len;
    const T *_data;
public:
    GenString(int len, const byte *data): _len(len),  _data(dup_data(len, data)) {}   
    ~GenString()  { delete[] _data; }
    int get_len() const  { return _len; }
    const T *get_data() const { return _data; }
};


typedef GenString<byte> ByteString;
typedef GenString<CHAR_TYPE> CharString;

#define CHAR_BUFFER_LEN 49
char *get_char_array(const CHAR_TYPE *sequence, int length, char *buffer); 
CHAR_TYPE *str_to_sequence(const char* cstring, CHAR_TYPE *sequence, int length);
std::string str_to_string(const CHAR_TYPE *cstring, int length);

void *my_malloc(size_t size);
void *my_calloc(size_t size, size_t number);

int my_itoalen(int n);
#define mputs(s) printf("%s", s)
#define mputc(c) printf("%c", c)

//inline bool boolize(int n) { return n != 0; }

#endif

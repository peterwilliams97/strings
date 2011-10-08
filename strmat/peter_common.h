#ifndef _PETER_COMMON_H_
#define _PETER_COMMON_H_

// Peter's definitions

typedef int BOOL;
#define FALSE 0
#define TRUE 1
#define ERROR 0
#define OK 1

#define ALPHABET_SIZE 256

typedef short CHAR_TYPE;


#define CHAR_BUFFER_LEN 129
char *get_char_array(const CHAR_TYPE *cstring, int length, char *buffer); 
CHAR_TYPE *str_to_sequence(const char* cstring, CHAR_TYPE *sequence, int length);

void *my_malloc(size_t size);
void *my_calloc(size_t size, size_t number);

#endif

#include <stdio.h>
#include <stdlib.h>
#include "strmat.h"
#include "strmat_alpha.h"

#define UNUSED 0
#define UNINITIALIZED 1
#define INITIALIZED 2

typedef struct {
  int init_flag, alpha_size;
  char rawmap[128], map[128];
} ALPHABET;

static ALPHABET *alpha_ascii = NULL;

int alpha_init()
{
    if (alpha_ascii == NULL) {
       if ((alpha_ascii = (ALPHABET *)calloc(sizeof(ALPHABET), 1)) == NULL) {
            fprintf(stderr, "Error: Ran out of memory.  Cannot initialize alphabet table\n");
            return 0;
        }
        alpha_ascii->init_flag = UNINITIALIZED;
    }

    if (alpha_ascii->init_flag == UNINITIALIZED) {
        int i;
        for (i=0; i < 128; i++) {
            alpha_ascii->rawmap[i] = alpha_ascii->map[i] = (char) i;
        }
        alpha_ascii->alpha_size = 128;
        alpha_ascii->init_flag = INITIALIZED;
    }
    return 1;
}

char rawmapchar(char ch)
{
    if (!alpha_init()) {
        return -1;
    }
    return alpha_ascii->rawmap[(int) ch];
}

char mapchar(char ch)
{
    if (!alpha_init()) {
        return -1;
    }  
    return alpha_ascii->map[(int) ch];
}  

int get_alpha_size()
{
    if (!alpha_init()) {
        return -1;
    }  
    return alpha_ascii->alpha_size;
}





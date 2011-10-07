#include <string.h>
#include <stdio.h>
#include <conio.h>
#include "strmat.h"
#include "strmat_util.h"
#include "stree_ukkonen.h"
#include "strmat_stubs2.h"


static int stree_print_flag = ON;
static int stats_flag = ON;

#define NUM_STRINGS 4
#define UNIQUE_STRINGS 2

static int test1()
{
    int i, ok = 1;
    STRING *strings[NUM_STRINGS];

    for (i = 0; i < NUM_STRINGS; i++) {
        char title[129];
        char cstring[257];
        char dstring[257];
        int length = 0;
        int j;
        sprintf(title, "string %02d", i+1);
 
        //sprintf(cstring, "hello_%02d,testing-%02d", i+1, i+1); 
        //sprintf(cstring, "ab%02dx", i+1);
        //sprintf(cstring, "%c%c%02d%c", 'a' + i, 'm' + i, i+1, 'x' + i);
        
        if (i % UNIQUE_STRINGS == 0) {
            sprintf(cstring, "abc");
        } else if (i % UNIQUE_STRINGS == 1) {
            sprintf(cstring, "ABC");
        } else if (i % UNIQUE_STRINGS == 2) {
            sprintf(cstring, "efg");
        } else if (i % UNIQUE_STRINGS == 3) {
            sprintf(cstring, "EFG");
        }
        strcpy(dstring, "xxx");
        strcat(dstring, cstring);
        strcat(dstring, "yyy");
           
        //printf("String %2d: '%s' '%s'\n", i, title, dstring);
#if 1
        length = 5;
        for (j = 0; j < length; j++) {
            dstring[j] = j + i % UNIQUE_STRINGS;
        }

        //length = strlen(dstring);
        //dstring[4] = 0;
        strings[i] = make_seqn(title, dstring, length);
#else
        strings[i] = make_seq(title, dstring);
#endif
    }

    ok = strmat_ukkonen_build(strings, NUM_STRINGS, stats_flag, stree_print_flag);

    if (!ok) {
        fprintf(stderr, "strmat_ukkonen_build failed\n");
        return 0;
    }

   printf("Print any key to exit...");
   _getch();
   return 1;
}

int main(int argc, char *argv[]) 
{
    test1();
}
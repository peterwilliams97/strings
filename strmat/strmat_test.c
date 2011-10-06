#include <string.h>
#include "strmat.h"
#include "strmat_util.h"
//#include "sary_match.h"
#include "stree_ukkonen.h"
#include "strmat_alpha.h"
#include "strmat_stubs2.h"

#define ON 1
#define OFF 0
static int ch_len;
static int stree_build_policy = SORTED_LIST;
static int stree_build_threshold = 10;
static int stree_print_flag = ON;
static int stats_flag = ON;

#define NUM_STRINGS 4
#define UNIQUE_STRINGS 2

int main(int argc, char *argv[])
{
    int i;
    STRING *strings[NUM_STRINGS];

    for (i = 0; i < NUM_STRINGS; i++) {
        char title[128];
        char cstring[256];
        char dstring[256];
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
           
        printf("String %2d: '%s' '%s'\n", i, title, dstring);
        strings[i] = make_seq(title, dstring);
    }

    strmat_ukkonen_build(strings, NUM_STRINGS, stree_build_policy,
                             stree_build_threshold, stats_flag, stree_print_flag);

   // getch();

}
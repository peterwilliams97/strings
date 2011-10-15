#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#include <string>
#include "strmat.h"
#include "strmat_util.h"
#include "stree_ukkonen.h"
#include "strmat_stubs2.h"
#include "peter_io.h"

using namespace std;

static int stree_print_flag = TRUE;
static int stats_flag = TRUE;

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
        CHAR_TYPE sequence[257];
        int length = 0;
        int j;
        sprintf(title, "string %02d", i+1);
      
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
           
#if 1
        length = 5;
        for (j = 0; j < length; j++) {
            dstring[j] = j + i % UNIQUE_STRINGS;
        }
        strings[i] = make_seqn(title, str_to_sequence(cstring, sequence, 257), length, stree_print_flag);
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
   return 1;
}

static STRING **make_test_strings(int num_strings, int num_unique, int length, int max_char)
{
    STRING **strings = (STRING **)my_calloc(num_strings, sizeof(STRING *));
    CHAR_TYPE *cstring = (CHAR_TYPE *)my_calloc(length, sizeof(CHAR_TYPE));
    char title[129];
    
    printf("make_test_strings(num_strings=%d, num_unique=%d, length=%d, max_char=%d)\n",
        num_strings, num_unique, length, max_char);

    for (int i = 0; i < num_strings; i++) {
        sprintf(title, "string %2d", i+1);
        for (int j = 0; j < length; j++) {
            cstring[j] = (j + i % num_unique) % max_char;
        }
        strings[i] = make_seqn(title, cstring, length, stree_print_flag);
    }
    free(cstring);
    return strings;
}

static void free_test_strings(int num_strings, STRING **strings)
{
    for (int i = 0; i < num_strings; i++) {
        free_seq(strings[i]);
    }
    free(strings);
}

static int num_calls = 0;
static bool base_test(int num_strings, int num_unique, int length, int max_char)
{
    STRING **strings = make_test_strings(num_strings, num_unique, length, max_char);
     
    printf("\n %3d: base_test(num_strings=%d, num_unique=%d, length=%d, max_char=%d)\n",
        num_calls++, num_strings, num_unique, length, max_char);

    bool ok = boolize(strmat_ukkonen_build(strings, num_strings, stats_flag, stree_print_flag));
    if (!ok) {
        fprintf(stderr, "strmat_ukkonen_build failed\n");
    }

    free_test_strings(num_strings, strings);  
    return ok;
}

static bool test2(int num_strings, int num_unique, int length, int max_char)
{
    return base_test(num_strings, num_unique, length, max_char);
}

static _int64 _last_val = 0;
static int range(int val, int min_val, int max_val)
{
    _last_val = (_last_val * 152700091 + val * 153102757) % 152500063;
    assert(_last_val >= 0);
    int result = min_val + _last_val % (max_val - min_val);
    assert(result >= 0);
    return result;
}

static BOOL test3()
{
    BOOL ok = TRUE;
    for (int i = 0; i < 10; i++) {
             
        int num_strings = range(i, 1, 100);
        int num_unique = range(i, 1, 50); 
        int length = range(i, 2, 1000);
        int max_char = range(i, 1, 255);

        if (!base_test(num_strings, num_unique, length, max_char)) {
            fprintf(stderr, "FAILURE !!!\n");
            ok = FALSE;
            break;
        }
    }
    return ok;
}

static BOOL test5()
{
    int num_strings = 4;
    int num_unique = 2; 
    int length = 200;
    int max_char = 155;
    BOOL ok = TRUE;
    
    for (int i = 0; i < 1000; i++) {
        stree_print_flag = (i == 999); 
        stats_flag = (i == 0) || stree_print_flag;
        if (!base_test(num_strings, num_unique, length, max_char)) {
            fprintf(stderr, "FAILURE !!!\n");
            ok = FALSE;
            break;
        }
    }
    return ok;
}

static string oki_dir("c:\\dev\\suffix\\find.page.markers\\hiperc\\");

static const char *oki_file_list[] = {
 "oki5650-pages=1-doc.prn",
 "oki5650-pages=1-pdf.prn",
 "oki5650-pages=17-pdf.prn",
 "oki5650-pages=2-doc.prn",
 "oki5650-pages=2-pdf.prn",
 "oki5650-pages=2.prn",
 "oki5650-pages=3-doc.prn",
 "oki5650-pages=3-pdf.prn",
 "pages=1-color-oki5100.prn",
 "pages=1-mono-oki5100.prn",
 "pages=2-blank-A4-landscape-oki5100.prn",
 "pages=2-blank-A4-oki5100.prn",
 "pages=2-blank-A5-oki5100.prn",
 "pages=4-blank-oki5100.prn",
 "pages=5-blank-oki5100.prn",
 "pages=5-simple-oki5100.prn"
};

static const int NUM_OKI_STRINGS = sizeof(oki_file_list)/sizeof(oki_file_list[0]);
static STRING **get_oki_file_strings()
{
    STRING **strings = (STRING **)my_calloc(sizeof(STRING *), NUM_OKI_STRINGS);

    for (int i = 0; i < NUM_OKI_STRINGS; i++) {
        string fname(oki_file_list[i]);
        string fpath;
        fpath = oki_dir + fname;
        cout << "path = " << fpath << endl;
        FileData file_data = read_file_data(fpath);
        strings[i] = make_seqn_from_bytes(fname.c_str(), file_data.get_data(), file_data.get_size(), stree_print_flag);
    }   
    return strings;
}

static int sortfunc(const void *p1, const void *p2)
{
    const STRING **s1 = (const STRING **)p1;
    const STRING **s2 = (const STRING **)p2;
    return +((*s1)->length - (*s2)->length);
}

static BOOL test6()
{
    STRING **strings = get_oki_file_strings();
    qsort(strings, NUM_OKI_STRINGS, sizeof(STRING*), sortfunc);
    
    BOOL ok = strmat_ukkonen_build(strings, NUM_OKI_STRINGS, stats_flag, stree_print_flag);
    if (!ok) {
        fprintf(stderr, "strmat_ukkonen_build failed\n");
    }
    for (int i = 0; i < NUM_OKI_STRINGS; i++) {
        free_seq(strings[i]);
    }
    free(strings);
    return ok;

}

void print_node(const STREE_NODE node)
{
    char buffer[CHAR_BUFFER_LEN];

    cout << "Node" << endl;
    cout << "   id=" << node->id;
    cout << "index=" << node->_index;
    cout << " edge=" << get_char_array(node->edgestr, node->edgelen, buffer) << "  " << node->edgelen << endl;
}

static void match_pattern(SUFFIX_TREE tree, CHAR_TYPE *pattern, int n)
{    
    STREE_NODE node;
    int pos = -1;
    memset(&node, 0, sizeof(node));

   int len = stree_match(tree, pattern, n, &node, &pos);

   char buffer[CHAR_BUFFER_LEN];
   cout << "Matched " << get_char_array(pattern, n, buffer) << endl;
   cout << "len  = " << len << endl;
   cout << "pos  = " << pos << endl;
   print_node(node);
}

int main(int argc, char *argv[]) 
{
    int test_num = 2;
    
    switch(test_num) {
    
    case 1:     // ASCII example
        test1(); 
        break;
    case 2:     // Simple binary example
        {
            int num_strings = 4;
            int num_unique = 2;
            int length = 6;
            int max_char = 4;
            test2(num_strings, num_unique, length, max_char);
        }
        break;
    case 3:     // Stress binary example
        stree_print_flag = FALSE;
        test3();
        break;
    case 4:     // big binary example
        stree_print_flag = FALSE;
        {
            int num_strings = 40;
            int num_unique = 20;
            int length = 10000;
            int max_char = 200;
            test2(num_strings, num_unique, length, max_char);
        }
        break;
    case 5:     // Many tests
        test5();
        break;
    case 6:     // Read binary strings from files
        stree_print_flag = FALSE;
        test6();
        break;
   }

    printf("Print any key to exit...");
    _getch();
}
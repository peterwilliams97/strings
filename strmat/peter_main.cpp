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
#include "strmat_print.h"

#include "peter_io.h"
#include "peter_longest_common_extension.h"

using namespace std;

static bool do_match = true;
static bool stree_print_flag = true;
static bool stats_flag = true;

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

    ok = strmat_ukkonen_build((const STRING **)strings, NUM_STRINGS, stats_flag, stree_print_flag);

    if (!ok) {
        fprintf(stderr, "strmat_ukkonen_build failed\n");
        return 0;
    }

   printf("Print any key to exit...");
   return 1;
}

static const STRING **make_test_strings(int num_strings, int num_unique, int length, int max_char)
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
    return (const STRING **)strings;
}

static const STRING *make_palindrome(int length, int pal_ofs, int pal_len)
{
    printf("make_palindrome(length=%d,pal_ofs=%d,pal_len=%d)\n",
        length, pal_ofs, pal_len);

    CHAR_TYPE *cstring = (CHAR_TYPE *)my_calloc(length, sizeof(CHAR_TYPE));
    for (int i = 0; i < length; i++) {
        cstring[i] = i % 100;
    }
    int center = pal_ofs + pal_len/2;
    for (int i = 0; i < pal_len/2; i++) {
        cstring[center - i] = 100 + i % 100;
        cstring[center + i] = 100 + i % 100;
    }

    char title[129];
    sprintf(title, "palindrome(ofs=%d,len=%d)", pal_ofs, pal_len);
    STRING *str = make_seqn(title, cstring, length, stree_print_flag);
    free(cstring);
    return (const STRING *)str;
}

static void free_test_strings(int num_strings, const STRING **strings)
{
    for (int i = 0; i < num_strings; i++) {
        free_seq((STRING *)strings[i]);
    }
    free((STRING **)strings);
}

static int num_calls = 0;
static bool base_construction_test(int num_strings, int num_unique, int length, int max_char)
{
    const STRING **strings = make_test_strings(num_strings, num_unique, length, max_char);
     
    printf("\n %3d: base_construction_test(num_strings=%d, num_unique=%d, length=%d, max_char=%d)\n",
        num_calls++, num_strings, num_unique, length, max_char);

    bool ok = strmat_ukkonen_build(strings, num_strings, stats_flag, stree_print_flag);
    if (!ok) {
        fprintf(stderr, "strmat_ukkonen_build failed\n");
    }

    free_test_strings(num_strings, strings);  
    return ok;
}

static bool base_match_test(int num_strings, int num_unique, int length, int max_char, int max_match)
{   
    const STRING **strings = make_test_strings(num_strings, num_unique, length, max_char);
    int match_len = (max_match >= 0) ? min(max_match,(length+1)/2) : (length+1)/2;  
    STRING *pattern = make_seqn("pattern", strings[0]->sequence + 2, match_len, stree_print_flag);
     
    printf("\n %3d: base_match_test(num_strings=%d, num_unique=%d, length=%d, max_char=%d)\n",
        num_calls++, num_strings, num_unique, length, max_char);

    bool ok = strmat_stree_match(pattern, strings, num_strings, stats_flag, stree_print_flag);
    if (!ok) {
        fprintf(stderr, "strmat_stree_match failed\n");
    }

    free_seq(pattern);
    free_test_strings(num_strings, strings);  
    return ok;
}

static bool base_test(int num_strings, int num_unique, int length, int max_char, int max_match = -1)
{
    if (do_match) {
        return base_match_test(num_strings, num_unique, length, max_char, max_match);
    } else {
        return base_construction_test(num_strings, num_unique, length, max_char);
    }
}

static bool test2(int num_strings, int num_unique, int length, int max_char, int max_match)
{
    return base_test(num_strings, num_unique, length, max_char, max_match);
}

static bool test2a(int num_strings, int num_unique, int length, int max_char)
{
    return base_match_test(num_strings, num_unique, length, max_char, -1);
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

static bool test3()
{
    bool ok = true;
    for (int i = 0; i < 10; i++) {
             
        int num_strings = range(i, 1, 100);
        int num_unique = range(i, 1, 50); 
        int length = range(i, 2, 1000);
        int max_char = range(i, 1, 255);

        if (!base_test(num_strings, num_unique, length, max_char)) {
            fprintf(stderr, "FAILURE !!!\n");
            ok = false;
            break;
        }
    }
    return ok;
}

static bool test5()
{
    int num_strings = 4;
    int num_unique = 2; 
    int length = 200;
    int max_char = 155;
    bool ok = true;
    
    for (int i = 0; i < 1000; i++) {
        stree_print_flag = (i == 999); 
        stats_flag = (i == 0) || stree_print_flag;
        if (!base_test(num_strings, num_unique, length, max_char)) {
            fprintf(stderr, "FAILURE !!!\n");
            ok = false;
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
static const STRING **get_oki_file_strings()
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
    return (const STRING **)strings;
}

static int sortfunc(const void *p1, const void *p2)
{
    const STRING **s1 = (const STRING **)p1;
    const STRING **s2 = (const STRING **)p2;
    return +((*s1)->length - (*s2)->length);
}

static bool test6()
{
    const STRING **strings = get_oki_file_strings();
    qsort(strings, NUM_OKI_STRINGS, sizeof(STRING*), sortfunc);
    
    bool ok = strmat_ukkonen_build(strings, NUM_OKI_STRINGS, stats_flag, stree_print_flag);
    if (!ok) {
        fprintf(stderr, "strmat_ukkonen_build failed\n");
    }

    free_test_strings(NUM_OKI_STRINGS, strings);
    return ok;
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

static void lce_test(int length, int max_char, int num_tests)
{
    const STRING **strings = make_test_strings(2, 2, length, max_char);
     
    printf("lce_test(length=%d, max_char=%d)\n", length, max_char);
    LCE *lce_ctx = prepare_longest_common_extension(strings[0], strings[1], true);

    int delta = (length + num_tests - 1)/num_tests;
    for (int i = 0; i < num_tests; i++) {
        int ofs1 = ((i+1) * delta) % length;
        int ofs2 = ((i+2) * delta) % length;
        printf("%4d: (ofs1=%3d,ofs2=%3d) ", i, ofs1, ofs2);
        STREE_NODE lce_node = lookup_lce(lce_ctx, ofs1, ofs2);
        cout << " => ";
        print_node(lce_node, "LCE");
        cout << endl;
    }
      

    longest_common_extension_free(lce_ctx);
    free_test_strings(2, strings);  
}

int main(int argc, char *argv[]) 
{
    int test_num = 10;
    
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
            int max_match = 2;
            test2(num_strings, num_unique, length, max_char, max_match);
            //test2a(num_strings, num_unique, length, max_char);
        }
        break;
    case 3:     // Stress binary example
        stree_print_flag = do_match;
        test3();
        break;
    case 4:     // big binary example
        stree_print_flag = do_match;
        {
            int num_strings = 40;
            int num_unique = 20;
            int length = 10000;
            int max_char = 200;
            int max_match = 10;
            test2(num_strings, num_unique, length, max_char, max_match);
        }
        break;
    case 5:     // Many tests
        test5();
        break;
    case 6:     // Read binary strings from files
        stree_print_flag = false;
        test6();
        break;
     case 7:     // Another simple binary example
        {
            int num_strings = 2;
            int num_unique = 2;
            int length = 6;
            int max_char = 3;
            int max_match = 2;
            test2(num_strings, num_unique, length, max_char, max_match);
            //test2a(num_strings, num_unique, length, max_char);
        }
        break;
    case 8:     // Longest common extension test
        {
            int length = 6;
            int max_char = 10;
            int num_tests = 10;
            lce_test(length, max_char, num_tests);
        }
        break;
    case 9:     // Longest common extension stress test
        {
            int length = 10000;
            int max_char = 255;
            int num_tests = 100;
            lce_test(length, max_char, num_tests);
        }
        break;
    case 10:    // Longest palindrome
        {
            //const STRING **strings = make_test_strings(1, 1, 9, 2);
            //const STRING *s = strings[0];
            int length = 9;
            int pal_ofs = 2;
            int pal_len = 4;
            const STRING *str = make_palindrome(length, pal_ofs, pal_len);
            SubString substring = find_longest_palindrome(str, true);
            free_seq((STRING *)str);
            cout << endl;
            cout << "Longest palindrome: offset=" << substring._offset 
                << ",length=" << substring._length << endl;
        }
        break;
    
       
   }

    printf("Print any key to exit...");
    _getch();
}
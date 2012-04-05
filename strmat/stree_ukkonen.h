#ifndef _STREE_UKKONEN_H_
#define _STREE_UKKONEN_H_

#include "strmat.h"
#include "stree_strmat.h"

bool stree_ukkonen_add_string(SUFFIX_TREE tree, CHAR_TYPE *S, int M, int strid);
SUFFIX_TREE stree_ukkonen_build(STRING *string);
SUFFIX_TREE stree_gen_ukkonen_build(const STRING **strings, int num_strings, bool *ok, bool print_flag);

#endif

#ifndef _STRMAT_UTIL_H_
#define _STRMAT_UTIL_H_

#include "peter_common.h"

STRING *make_seq(const char *title, const CHAR_TYPE *cstring);
STRING *make_seqn(const char *title, const CHAR_TYPE *cstring, int length, BOOL print_flag);
void free_seq(STRING *sptr); 

#endif

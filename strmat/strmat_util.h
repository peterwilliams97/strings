#ifndef _STRMAT_UTIL_H_
#define _STRMAT_UTIL_H_

#include "peter_common.h"

STRING *make_seq(const char *title, const CHAR_TYPE *cstring);
STRING *make_seqn(const char *title, const CHAR_TYPE *cstring, int length, BOOL print_flag);
STRING *make_seqn_from_bytes(const char *title, const byte* bytes, int length, BOOL print_flag);
void free_seq(STRING *sptr); 

double mb(_int64 n);

#endif

#ifndef _STRMAT_UTIL_H_
#define _STRMAT_UTIL_H_

int my_itoalen(int n);
#define mputs(s) printf("%s", s)
#define mputc(c) printf("%c", c)

STRING *make_seq(const char *title, const CHAR_TYPE *cstring);
STRING *make_seqn(const char *title, const CHAR_TYPE *cstring, int length);
void free_seq(STRING *sptr); 

#endif

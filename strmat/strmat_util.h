int my_itoalen(int n);
#define mputs(s) printf("%s", s)
#define mputc(c) printf("%c", c)


#define CHAR_BUFFER_LEN 129
char *get_char_array(const CHAR_TYPE *cstring, int length, char *buffer); 

STRING *make_seq(const char *title, const CHAR_TYPE *cstring);
STRING *make_seqn(const char *title, const char *CHAR_TYPE, int length);
void free_seq(STRING *sptr); 


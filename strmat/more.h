#include <stdio.h>
//void mstart(FILE *fp_in, FILE *fp_out, int go_flag, int setup_tty_flag, int init_lines, void (*quit_fn)());
//void mend(int follow_lines);
//int mputc(char ch), mputs(char *s), mprintf(char *format, ...);

#define mstart(fp_in, fp_out, go_flag, setup_tty_flag, init_lines, quit_fn)
#define mputs(s) printf("%s", s)
#define mputc(c) printf("%c", c)
#define mprintf printf
#define mend(i)

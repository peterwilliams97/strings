#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "strmat.h"
#include "strmat_alpha.h"
#include "strmat_print.h"
//#include "strmat_seqary.h"
#include "strmat_util.h"
//#include "sary_match.h"
#include "stree_ukkonen.h"

#define ON 1
#define OFF 0

static char *choice;
static int ch_len;
static int stree_build_policy = SORTED_LIST;
static int stree_build_threshold = 10;
static int stree_print_flag = ON;
static int stats_flag = ON;

//FILE     *fpout = (FILE *)1; // stdout; // !@#$

/*
 * Forward prototypes.
 */
//void util_menu(void);
//void basic_alg_menu(void);
//void z_alg_menu(void);
//void suf_tree_menu(void);
//void suf_ary_menu(void);
//void repeats_menu(void);
//void set_display_options(void);

int my_itoalen(int num)
{
    int i;

    for (i=1; num >= 10; i++)
        num /= 10;
    return i;
}

char *my_getline(FILE *fp, int *len_out)
{
  static int bufsize = 0;
  static char *buffer = NULL;
  int size, len;
  char *s;

  if (!buffer) {
    bufsize = 20;
    if ((buffer = malloc(bufsize)) == NULL) {
      fprintf(stderr, "Memory Error:  Ran out of memory.\n");
      return NULL;
    }
  }  

  s = buffer + bufsize - 2;
  *s = '\0';
  if (fgets(buffer, bufsize, fp) == NULL)
    return NULL;
  else if (!*s || *s == '\n') {
    len = strlen(buffer);
    if (buffer[len-1] == '\n')
      buffer[--len] = '\0';
    if (len_out) *len_out = len;
    return buffer;
  }

  while (1) {
    size = bufsize - 1;
    bufsize += bufsize;
    if ((buffer = realloc(buffer, bufsize)) == NULL) {
      fprintf(stderr, "Memory Error:  Ran out of memory.\n");
      return NULL;
    }

    s = buffer + bufsize - 2;
    *s = '\0';
    if (fgets(buffer + size, bufsize - size, fp) == NULL) {
      len = size;
      if (buffer[len-1] == '\n')
        buffer[--len] = '\0';
      if (len_out) *len_out = len;
      return buffer;
    }
    else if (!*s || *s == '\n') {
      len = size + strlen(buffer + size);
      if (buffer[len-1] == '\n')
        buffer[--len] = '\0';
      if (len_out) *len_out = len;
      return buffer;
    }
  }
}


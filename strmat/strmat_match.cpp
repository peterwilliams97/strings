#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "strmat_match.h"

static MATCHES freelist = NULL;

MATCH_NODE *alloc_match(void)
{
  MATCH_NODE *temp;

  if (!freelist) {
    if ((temp = (MATCH_NODE *)my_malloc(sizeof(MATCH_NODE))) == NULL)
      return NULL;
  }
  else {
    temp = freelist;
    freelist = freelist->next;
  }

  memset(temp, 0, sizeof(MATCH_NODE));
  return temp;
}

void free_matches(MATCHES list)
{
  MATCHES last;

  if (list == NULL)
    return;

  for (last=list; last->next != NULL; last=last->next) ;
  last->next = freelist;
  freelist = list;
}

int print_matches(const STRING *string, const STRING **strings, int num_strings, MATCHES list, int num_matches)
{
    int i, j, N, count, matchdigs, minwidth, maxwidth;
    int width, maxposdigs, maxiddigs, multistring_mode;
    CHAR_TYPE *T;
    char format[32];
    MATCHES ptr;

    if (num_matches == 0) {
        mputs("Found 0 matches:\n");
        return 1;
    }

    multistring_mode = (list->type == TEXT_SET_EXACT);
    if ((!multistring_mode && string == NULL) 
        || (multistring_mode && strings == NULL))
    return 0;

  /*
   * Compute the number of digits in the number of matches and the
   * widest position (and possibly widest identifier) of any match.  Also,
   * find the smallest and largest match length.
   */
  matchdigs = my_itoalen(num_matches);

  maxposdigs = maxiddigs = 0;
  minwidth = maxwidth = -1;
  for (count=0,ptr=list; count < num_matches; count++,ptr=ptr->next) {
    if (minwidth == -1 || ptr->rend - ptr->lend + 1 < minwidth)
      minwidth = ptr->rend - ptr->lend + 1;
    if (maxwidth == -1 || ptr->rend - ptr->lend + 1 > maxwidth)
      maxwidth = ptr->rend - ptr->lend + 1;

    width = my_itoalen(ptr->rend);
    if (width > maxposdigs)
      maxposdigs = width;

    if (multistring_mode && num_strings > 1) {
      width = my_itoalen(ptr->textid);
      if (width > maxiddigs)
        maxiddigs = width;
    }
  }

  /* 
   * Print the header line with signal characters showing where
   * the matches are.
   */
  printf("Found %d matches:", num_matches);
  width = (17 + 2 * maxposdigs + maxiddigs) - (15 + matchdigs);
  for (i=0; i < width; i++)
    mputc(' ');
  for (i=0; i < minwidth && i < 43; i++)
    mputc('_');
  for ( ; i < maxwidth && i < 43; i++)
    mputc('.');
  if (i == 43)
    mputc('>');
  mputc('\n');

  /*
   * Build the format for printing the positions of the match, and 
   * then loop through the matches, printing the information.
   */
  if (multistring_mode && num_strings > 1)
    sprintf(format, " %%%dd:  %%%dd-%%%dd:    ", maxiddigs, maxposdigs,
            maxposdigs);
  else
    sprintf(format, "    %%%dd-%%%dd:    ", maxposdigs, maxposdigs);

  T = NULL;
  N = 0;
  if (!multistring_mode) {
      T = string->sequence;
      N = string->length;
  }
  else if (num_strings == 1) {
     T = strings[0]->sequence;
    N = strings[0]->length;
  }
    
  for (count=0,ptr=list; count < num_matches; count++,ptr=ptr->next) {
    if (multistring_mode && num_strings > 1) {
      printf(format, ptr->textid, ptr->lend, ptr->rend);
      T = strings[ptr->textid-1]->sequence;
      N = strings[ptr->textid-1]->length;
    }
    else
      printf(format, ptr->lend, ptr->rend);

    if (ptr->lend - 4 >= 1) {
      mputs("...");
      i = ptr->lend - 4;
    }
    else {
      mputs("   ");
      for (i=ptr->lend-4; i < 1; i++)
        mputc(' ');
    }

    for (j=0; j < 44 && i <= N && i <= ptr->rend + 4; i++,j++)
      mputc((isprint((int)T[i-1]) ? T[i-1] : '#'));

    if (i <= N)
      mputs("...");

    if (mputc('\n') == 0)
      break;
  }

  mputc('\n');

  return 1;
}

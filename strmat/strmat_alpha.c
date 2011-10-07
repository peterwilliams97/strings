#include <stdio.h>
#include <stdlib.h>
#include "strmat.h"
#include "strmat_alpha.h"

#define UNUSED 0
#define UNINITIALIZED 1
#define INITIALIZED 2

typedef struct {
  int init_flag, alpha_size;
  char rawmap[128], map[128];
} ALPHABET;

static ALPHABET *alpha_ascii = NULL;

int alpha_init()
{
    if (alpha_ascii == NULL) {
       if ((alpha_ascii = (ALPHABET *)calloc(sizeof(ALPHABET), 1)) == NULL) {
            fprintf(stderr, "Error: Ran out of memory.  Cannot initialize alphabet table\n");
            return 0;
        }
        alpha_ascii->init_flag = UNINITIALIZED;
    }

    if (alpha_ascii->init_flag == UNINITIALIZED) {
        int i;
        for (i=0; i < 128; i++) {
            alpha_ascii->rawmap[i] = alpha_ascii->map[i] = (char) i;
        }
        alpha_ascii->alpha_size = 128;
        alpha_ascii->init_flag = INITIALIZED;
    }
    return 1;
}

char rawmapchar(char ch)
{
    if (!alpha_init()) {
        return -1;
    }
    return alpha_ascii->rawmap[(int) ch];
}

char mapchar(char ch)
{
    if (!alpha_init()) {
        return -1;
    }  
    return alpha_ascii->map[(int) ch];
}  

int get_alpha_size()
{
    if (!alpha_init()) {
        return -1;
    }  
    return alpha_ascii->alpha_size;
}

//int add_alphabet(char *characters)
//{
//  int i, j, alpha;
//  char *s;
//  ALPHABET *tptr;
//
//  if (alpha_tables == NULL)
//    if (!alpha_init(-1))
//      return -1;
//
//  for (alpha=1; alpha < table_size; alpha++)
//    if (alpha_tables[alpha].init_flag == UNUSED)
//      break;
//
//  if (alpha == table_size) {
//    table_size += table_size;
//    if ((alpha_tables = realloc(alpha_tables,
//                                table_size * sizeof(ALPHABET))) == NULL) {
//      fprintf(stderr, "Ran out of memory.  Cannot create new alphabet.\n");
//      return -1;
//    }
//
//    for (i=alpha; i < table_size; i++)
//      alpha_tables[i].init_flag = UNUSED;
//  }
//
//  tptr = &alpha_tables[alpha];
//  for (i=0; i < 128; i++) {
//    tptr->rawmap[i] = 0;
//    tptr->map[i] = -1;
//  }
//  for (s=characters; *s; s++)
//    tptr->rawmap[(int) *s] = *s;
//  for (i=0,j=0; i < 128; i++)
//    if (tptr->rawmap[i] != '\0')
//      tptr->map[i] = (char) j++;
//
//  tptr->alpha_size = j;
//  tptr->init_flag = INITIALIZED;
//
//  return alpha;
//}

//int map_sequences(STRING *s1, STRING *s2, STRING **sary, int arylen)
//{
//    int i, j, num, same, alpha;
//    char *s, *t, map[128], characters[129];
//    ALPHABET *tptr;
//
//  /*
//   * Check the simple case where all have the same alphabet.
//   */
//  same = 1;
//  alpha = ALPHA_UNKNOWN;
//  if (s1 != NULL)
//    alpha = s1->raw_alpha;
//  if (s2 != NULL) {
//    if (alpha == ALPHA_UNKNOWN)
//      alpha = s2->raw_alpha;
//    else if (alpha != s2->raw_alpha)
//      same = 0;
//  }
//  if (sary != NULL) {
//    for (i=0; i < arylen; i++) {
//      if (sary[i] != NULL) {
//        if (alpha == ALPHA_UNKNOWN)
//          alpha = sary[i]->raw_alpha;
//        else if (alpha != sary[i]->raw_alpha)
//          same = 0;
//      }
//    }
//  }
//
//  /*
//   * If not, print a warning message and create a new alphabet formed
//   * by the combination of all the characters in the alphabets.
//   */
//  if (!same) {
//    fprintf(stderr, "Warning: Alphabets are not identical...\n");
//
//    for (i=0; i < 128; i++)
//      map[i] = '\0';
//
//    if (s1 != NULL) {
//      tptr = &alpha_tables[s1->raw_alpha];
//      for (i=0; i < 128; i++)
//        if (tptr->rawmap[i] != '\0')
//          map[(int) tptr->rawmap[i]] = 1;
//    }
//    if (s2 != NULL) {
//      tptr = &alpha_tables[s2->raw_alpha];
//      for (i=0; i < 128; i++)
//        if (tptr->rawmap[i] != '\0')
//          map[(int) tptr->rawmap[i]] = 1;
//    }
//    if (sary != NULL) {
//      for (num=0; num < arylen; num++) {
//        if (sary[num] != NULL) {
//          tptr = &alpha_tables[sary[num]->raw_alpha];
//          for (i=0; i < 128; i++)
//            if (tptr->rawmap[i] != '\0')
//              map[(int) tptr->rawmap[i]] = 1;
//        }
//      }
//    }
//
//    for (i=1,j=0; i < 128; i++)
//      if (map[i] == 1)
//        characters[j++] = (char) i;
//    characters[j] = '\0';
//
//    if ((alpha = add_alphabet(characters)) == -1)
//      return -1;
//  }
//
//  /*
//   * Map the sequences to the new alphabet.
//   */
//  if (s1 != NULL && (!same || s1->alphabet != alpha)) {
//    s1->alphabet = alpha;
//    s1->alpha_size = alpha_tables[alpha].alpha_size;
//    for (i=0,s=s1->sequence,t=s1->raw_seq; i < s1->length; i++,s++,t++)
//      if ((*s = mapchar(alpha, *t)) == -1) {
//        fprintf(stderr,
//                "\nError: Sequence %s contains unmappable characters.\n",
//                s1->title);
//        return -1;
//      }
//  }
//  if (s2 != NULL && (!same || s2->alphabet != alpha)) {
//    s2->alphabet = alpha;
//    s2->alpha_size = alpha_tables[alpha].alpha_size;
//    for (i=0,s=s2->sequence,t=s2->raw_seq; i < s2->length; i++,s++,t++)
//      if ((*s = mapchar(alpha, *t)) == -1) {
//        fprintf(stderr,
//                "\nError: Sequence %s contains unmappable characters.\n",
//                s2->title);
//        return -1;
//      }
//  }
//  if (sary != NULL) {
//    for (num=0; num < arylen; num++) {
//      if (sary[num] != NULL && (!same || sary[num]->alphabet != alpha)) {
//        sary[num]->alphabet = alpha;
//        sary[num]->alpha_size = alpha_tables[alpha].alpha_size;
//        s = sary[num]->sequence;
//        for (i=0,t=sary[num]->raw_seq; i < sary[num]->length; i++) 
//          if ((*s++ = mapchar(alpha, *t++)) == -1) {
//            fprintf(stderr,
//                    "\nError: Sequence %s contains unmappable characters.\n",
//                    sary[num]->title);
//            return -1;
//          }
//      }
//    }
//  }
//
//  return alpha;
//}
//
//
//
//void unmap_sequences(STRING *s1, STRING *s2, STRING **sary, int arylen)
//{
//  int i, alpha, keepflag;
//
//  /*
//   * Get the mapped alphabet.
//   */
//  alpha = ALPHA_UNKNOWN;
//  if (s1 != NULL)
//    alpha = s1->alphabet;
//  else if (s2 != NULL)
//    alpha = s2->alphabet;
//  else if (sary != NULL) {
//    for (i=0; i < arylen; i++) {
//      if (sary[i] != NULL) {
//        alpha = sary[i]->alphabet;
//        break;
//      }
//    }
//  }
//
//  if (alpha == ALPHA_UNKNOWN)
//    return;
//
//  /*
//   * Check to see whether the alphabet is to be freed (i.e., is a 
//   * user defined alphabet that is not a raw_alpha value for any of
//   * the sequences).
//   */
//  if (alpha <= NUM_PREDEF_ALPHA)
//    return;
//
//  keepflag = 0;
//  if (s1 != NULL) {
//    if (alpha == s1->raw_alpha)
//      keepflag = 1;
//  }
//  else if (s2 != NULL) {
//    if (alpha == s2->raw_alpha)
//      keepflag = 1;
//  }
//  else if (sary != NULL) {
//    for (i=0; i < arylen; i++) {
//      if (sary[i] != NULL) {
//        if (alpha == sary[i]->raw_alpha)
//          keepflag = 1;
//      }
//    }
//  }
//
//  if (keepflag)
//    return;
//
//  /*
//   * Free the alphabet and reset values.
//   */
//  alpha_free(alpha);
//  if (s1 != NULL)
//    s1->alphabet = ALPHA_UNKNOWN;
//  else if (s2 != NULL)
//    s2->alphabet = ALPHA_UNKNOWN;
//  else if (sary != NULL) {
//    for (i=0; i < arylen; i++)
//      if (sary[i] != NULL)
//        sary[i]->alphabet = ALPHA_UNKNOWN;
//  }
//}
//

void alpha_free()
{
  
}





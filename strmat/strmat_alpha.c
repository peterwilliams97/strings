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

int table_size;
ALPHABET *alpha_tables = NULL;


int alpha_init(int alpha)
{
    int i;
    ALPHABET *tptr;

    if (alpha_tables == NULL) {
        table_size = NUM_PREDEF_ALPHA + 1;
        if ((alpha_tables = malloc(table_size * sizeof(ALPHABET))) == NULL) {
            fprintf(stderr, "Error: Ran out of memory.  Cannot initialize alphabet table\n");
        return 0;
        }
        alpha_tables[ALPHA_UNKNOWN].init_flag = UNUSED;
        alpha_tables[ALPHA_DNA].init_flag = UNINITIALIZED;
        alpha_tables[ALPHA_RNA].init_flag = UNINITIALIZED;
        alpha_tables[ALPHA_PROTEIN].init_flag = UNINITIALIZED;
        alpha_tables[ALPHA_ASCII].init_flag = UNINITIALIZED;
    }

  if (alpha > 0 && alpha <= NUM_PREDEF_ALPHA &&
      alpha_tables[alpha].init_flag == UNINITIALIZED) {
    switch (alpha) {
    case ALPHA_DNA:
      tptr = &alpha_tables[alpha];
      for (i=0; i < 128; i++) {
        tptr->rawmap[i] = 0;
        tptr->map[i] = -1;
      }

      tptr->rawmap['a'] = tptr->rawmap['A'] = 'A';
      tptr->rawmap['c'] = tptr->rawmap['C'] = 'C';
      tptr->rawmap['g'] = tptr->rawmap['G'] = 'G';
      tptr->rawmap['t'] = tptr->rawmap['T'] = 'T';
      tptr->map['a'] = tptr->map['A'] = 0;
      tptr->map['c'] = tptr->map['C'] = 1;
      tptr->map['g'] = tptr->map['G'] = 2;
      tptr->map['t'] = tptr->map['T'] = 3;

      tptr->alpha_size = 4;
      tptr->init_flag = INITIALIZED;
      break;

    case ALPHA_RNA:
      tptr = &alpha_tables[alpha];
      for (i=0; i < 128; i++) {
        tptr->rawmap[i] = 0;
        tptr->map[i] = -1;
      }

      tptr->rawmap['a'] = tptr->rawmap['A'] = 'A';
      tptr->rawmap['c'] = tptr->rawmap['C'] = 'C';
      tptr->rawmap['g'] = tptr->rawmap['G'] = 'G';
      tptr->rawmap['u'] = tptr->rawmap['U'] = 'U';
      tptr->map['a'] = tptr->map['A'] = 0;
      tptr->map['c'] = tptr->map['C'] = 1;
      tptr->map['g'] = tptr->map['G'] = 2;
      tptr->map['u'] = tptr->map['U'] = 3;

      tptr->alpha_size = 4;
      tptr->init_flag = INITIALIZED;
      break;

    case ALPHA_PROTEIN:
      tptr = &alpha_tables[alpha];
      for (i=0; i < 128; i++) {
        tptr->rawmap[i] = 0;
        tptr->map[i] = -1;
      }

      tptr->rawmap['a'] = tptr->rawmap['A'] = 'A';
      tptr->rawmap['c'] = tptr->rawmap['C'] = 'C';
      tptr->rawmap['d'] = tptr->rawmap['D'] = 'D';
      tptr->rawmap['e'] = tptr->rawmap['E'] = 'E';
      tptr->rawmap['f'] = tptr->rawmap['F'] = 'F';
      tptr->rawmap['g'] = tptr->rawmap['G'] = 'G';
      tptr->rawmap['h'] = tptr->rawmap['H'] = 'H';
      tptr->rawmap['i'] = tptr->rawmap['I'] = 'I';
      tptr->rawmap['k'] = tptr->rawmap['K'] = 'K';
      tptr->rawmap['l'] = tptr->rawmap['L'] = 'L';
      tptr->rawmap['m'] = tptr->rawmap['M'] = 'M';
      tptr->rawmap['n'] = tptr->rawmap['N'] = 'N';
      tptr->rawmap['p'] = tptr->rawmap['P'] = 'P';
      tptr->rawmap['q'] = tptr->rawmap['Q'] = 'Q';
      tptr->rawmap['r'] = tptr->rawmap['R'] = 'R';
      tptr->rawmap['s'] = tptr->rawmap['S'] = 'S';
      tptr->rawmap['t'] = tptr->rawmap['T'] = 'T';
      tptr->rawmap['v'] = tptr->rawmap['V'] = 'V';
      tptr->rawmap['w'] = tptr->rawmap['W'] = 'W';
      tptr->rawmap['y'] = tptr->rawmap['Y'] = 'Y';
      tptr->map['a'] = tptr->map['A'] = 0;
      tptr->map['c'] = tptr->map['C'] = 1;
      tptr->map['d'] = tptr->map['D'] = 2;
      tptr->map['e'] = tptr->map['E'] = 3;
      tptr->map['f'] = tptr->map['F'] = 4;
      tptr->map['g'] = tptr->map['G'] = 5;
      tptr->map['h'] = tptr->map['H'] = 6;
      tptr->map['i'] = tptr->map['I'] = 7;
      tptr->map['k'] = tptr->map['K'] = 8;
      tptr->map['l'] = tptr->map['L'] = 9;
      tptr->map['m'] = tptr->map['M'] = 10;
      tptr->map['n'] = tptr->map['N'] = 11;
      tptr->map['p'] = tptr->map['P'] = 12;
      tptr->map['q'] = tptr->map['Q'] = 13;
      tptr->map['r'] = tptr->map['R'] = 14;
      tptr->map['s'] = tptr->map['S'] = 15;
      tptr->map['t'] = tptr->map['T'] = 16;
      tptr->map['v'] = tptr->map['V'] = 17;
      tptr->map['w'] = tptr->map['W'] = 18;
      tptr->map['y'] = tptr->map['Y'] = 19;

      tptr->alpha_size = 20;
      tptr->init_flag = INITIALIZED;
      break;

    case ALPHA_ASCII:
      tptr = &alpha_tables[alpha];
      for (i=0; i < 128; i++)
        tptr->rawmap[i] = tptr->map[i] = (char) i;

      tptr->alpha_size = 128;
      tptr->init_flag = INITIALIZED;
      break;
      
    default:
      fprintf(stderr, "Error:  The program is attempting to initialize "
              "an invalid alphabet.\n");
      return 0;
    }
  }
    return 1;
}

char rawmapchar(int alpha, char ch)
{
    if (alpha_tables == NULL) {
        if (!alpha_init(-1))
            return -1;
    }

    if (alpha < 1 || alpha >= table_size) {
        fprintf(stderr, "Error:  The program is using an invalid alphabet.\n");
        return -1;
    }

    if (alpha_tables[alpha].init_flag == UNINITIALIZED) {
        if (!alpha_init(alpha))
            return -1;
    }

    if (alpha_tables[alpha].init_flag == UNUSED) {
        fprintf(stderr, "Error:  The program is using an invalid alphabet.\n");
        return -1;
    }

    return alpha_tables[alpha].rawmap[(int) ch];
}

char mapchar(int alpha, char ch)
{
  if (alpha_tables == NULL)
    if (!alpha_init(-1))
      return -1;

  if (alpha < 1 || alpha >= table_size) {
    fprintf(stderr, "Error:  The program is using an invalid alphabet.\n");
    return -1;
  }

  if (alpha_tables[alpha].init_flag == UNINITIALIZED)
    if (!alpha_init(alpha))
      return -1;

  if (alpha_tables[alpha].init_flag == UNUSED) {
    fprintf(stderr, "Error:  The program is using an invalid alphabet.\n");
    return -1;
  }

  return alpha_tables[alpha].map[(int) ch];
}  

int get_alpha_size(int alpha)
{
  if (alpha_tables == NULL)
    if (!alpha_init(-1))
      return -1;

  if (alpha < 1 || alpha >= table_size) {
    fprintf(stderr, "Error:  The program is using an invalid alphabet.\n");
    return -1;
  }

  if (alpha_tables[alpha].init_flag == UNINITIALIZED)
    if (!alpha_init(alpha))
      return -1;

  if (alpha_tables[alpha].init_flag == UNUSED) {
    fprintf(stderr, "Error:  The program is using an invalid alphabet.\n");
    return -1;
  }

  return alpha_tables[alpha].alpha_size;
}


int add_alphabet(char *characters)
{
  int i, j, alpha;
  char *s;
  ALPHABET *tptr;

  if (alpha_tables == NULL)
    if (!alpha_init(-1))
      return -1;

  for (alpha=1; alpha < table_size; alpha++)
    if (alpha_tables[alpha].init_flag == UNUSED)
      break;

  if (alpha == table_size) {
    table_size += table_size;
    if ((alpha_tables = realloc(alpha_tables,
                                table_size * sizeof(ALPHABET))) == NULL) {
      fprintf(stderr, "Ran out of memory.  Cannot create new alphabet.\n");
      return -1;
    }

    for (i=alpha; i < table_size; i++)
      alpha_tables[i].init_flag = UNUSED;
  }

  tptr = &alpha_tables[alpha];
  for (i=0; i < 128; i++) {
    tptr->rawmap[i] = 0;
    tptr->map[i] = -1;
  }
  for (s=characters; *s; s++)
    tptr->rawmap[(int) *s] = *s;
  for (i=0,j=0; i < 128; i++)
    if (tptr->rawmap[i] != '\0')
      tptr->map[i] = (char) j++;

  tptr->alpha_size = j;
  tptr->init_flag = INITIALIZED;

  return alpha;
}


int map_sequences(STRING *s1, STRING *s2, STRING **sary, int arylen)
{
  int i, j, num, same, alpha;
  char *s, *t, map[128], characters[129];
  ALPHABET *tptr;

  /*
   * Check the simple case where all have the same alphabet.
   */
  same = 1;
  alpha = ALPHA_UNKNOWN;
  if (s1 != NULL)
    alpha = s1->raw_alpha;
  if (s2 != NULL) {
    if (alpha == ALPHA_UNKNOWN)
      alpha = s2->raw_alpha;
    else if (alpha != s2->raw_alpha)
      same = 0;
  }
  if (sary != NULL) {
    for (i=0; i < arylen; i++) {
      if (sary[i] != NULL) {
        if (alpha == ALPHA_UNKNOWN)
          alpha = sary[i]->raw_alpha;
        else if (alpha != sary[i]->raw_alpha)
          same = 0;
      }
    }
  }

  /*
   * If not, print a warning message and create a new alphabet formed
   * by the combination of all the characters in the alphabets.
   */
  if (!same) {
    fprintf(stderr, "Warning: Alphabets are not identical...\n");

    for (i=0; i < 128; i++)
      map[i] = '\0';

    if (s1 != NULL) {
      tptr = &alpha_tables[s1->raw_alpha];
      for (i=0; i < 128; i++)
        if (tptr->rawmap[i] != '\0')
          map[(int) tptr->rawmap[i]] = 1;
    }
    if (s2 != NULL) {
      tptr = &alpha_tables[s2->raw_alpha];
      for (i=0; i < 128; i++)
        if (tptr->rawmap[i] != '\0')
          map[(int) tptr->rawmap[i]] = 1;
    }
    if (sary != NULL) {
      for (num=0; num < arylen; num++) {
        if (sary[num] != NULL) {
          tptr = &alpha_tables[sary[num]->raw_alpha];
          for (i=0; i < 128; i++)
            if (tptr->rawmap[i] != '\0')
              map[(int) tptr->rawmap[i]] = 1;
        }
      }
    }

    for (i=1,j=0; i < 128; i++)
      if (map[i] == 1)
        characters[j++] = (char) i;
    characters[j] = '\0';

    if ((alpha = add_alphabet(characters)) == -1)
      return -1;
  }

  /*
   * Map the sequences to the new alphabet.
   */
  if (s1 != NULL && (!same || s1->alphabet != alpha)) {
    s1->alphabet = alpha;
    s1->alpha_size = alpha_tables[alpha].alpha_size;
    for (i=0,s=s1->sequence,t=s1->raw_seq; i < s1->length; i++,s++,t++)
      if ((*s = mapchar(alpha, *t)) == -1) {
        fprintf(stderr,
                "\nError: Sequence %s contains unmappable characters.\n",
                s1->title);
        return -1;
      }
  }
  if (s2 != NULL && (!same || s2->alphabet != alpha)) {
    s2->alphabet = alpha;
    s2->alpha_size = alpha_tables[alpha].alpha_size;
    for (i=0,s=s2->sequence,t=s2->raw_seq; i < s2->length; i++,s++,t++)
      if ((*s = mapchar(alpha, *t)) == -1) {
        fprintf(stderr,
                "\nError: Sequence %s contains unmappable characters.\n",
                s2->title);
        return -1;
      }
  }
  if (sary != NULL) {
    for (num=0; num < arylen; num++) {
      if (sary[num] != NULL && (!same || sary[num]->alphabet != alpha)) {
        sary[num]->alphabet = alpha;
        sary[num]->alpha_size = alpha_tables[alpha].alpha_size;
        s = sary[num]->sequence;
        for (i=0,t=sary[num]->raw_seq; i < sary[num]->length; i++) 
          if ((*s++ = mapchar(alpha, *t++)) == -1) {
            fprintf(stderr,
                    "\nError: Sequence %s contains unmappable characters.\n",
                    sary[num]->title);
            return -1;
          }
      }
    }
  }

  return alpha;
}



void unmap_sequences(STRING *s1, STRING *s2, STRING **sary, int arylen)
{
  int i, alpha, keepflag;

  /*
   * Get the mapped alphabet.
   */
  alpha = ALPHA_UNKNOWN;
  if (s1 != NULL)
    alpha = s1->alphabet;
  else if (s2 != NULL)
    alpha = s2->alphabet;
  else if (sary != NULL) {
    for (i=0; i < arylen; i++) {
      if (sary[i] != NULL) {
        alpha = sary[i]->alphabet;
        break;
      }
    }
  }

  if (alpha == ALPHA_UNKNOWN)
    return;

  /*
   * Check to see whether the alphabet is to be freed (i.e., is a 
   * user defined alphabet that is not a raw_alpha value for any of
   * the sequences).
   */
  if (alpha <= NUM_PREDEF_ALPHA)
    return;

  keepflag = 0;
  if (s1 != NULL) {
    if (alpha == s1->raw_alpha)
      keepflag = 1;
  }
  else if (s2 != NULL) {
    if (alpha == s2->raw_alpha)
      keepflag = 1;
  }
  else if (sary != NULL) {
    for (i=0; i < arylen; i++) {
      if (sary[i] != NULL) {
        if (alpha == sary[i]->raw_alpha)
          keepflag = 1;
      }
    }
  }

  if (keepflag)
    return;

  /*
   * Free the alphabet and reset values.
   */
  alpha_free(alpha);
  if (s1 != NULL)
    s1->alphabet = ALPHA_UNKNOWN;
  else if (s2 != NULL)
    s2->alphabet = ALPHA_UNKNOWN;
  else if (sary != NULL) {
    for (i=0; i < arylen; i++)
      if (sary[i] != NULL)
        sary[i]->alphabet = ALPHA_UNKNOWN;
  }
}


void alpha_free(int alpha)
{
  if (alpha > NUM_PREDEF_ALPHA && alpha < table_size)
    alpha_tables[alpha].init_flag = UNUSED;
}





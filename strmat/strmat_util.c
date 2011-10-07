#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "strmat.h"
#include "strmat_util.h"


int my_itoalen(int num)
{
    int i;

    for (i=1; num >= 10; i++)
        num /= 10;
    return i;
}

#define CHAR_BUFFER_LEN 129
char *get_char_array(const char *cstring, int length, char *buffer) 
{
    int i;
 
    buffer[0] = 0;
    for (i = 0; i < length; i++) {
        char s[20];
        sprintf(s, "x%02x", (unsigned int)cstring[i]);
        if (strlen(buffer) + strlen(s) + 1 >= CHAR_BUFFER_LEN) {
            break;
        }
        strcat(buffer, s);
    } 
    return buffer;
}

/********************************************************************** 
 *  Function: make_seq() by Peter                                           
 *                                                                    
 *  Parameter:                                                        
 *                                                                    
 * Convert a character array to a STRING
 * 
 **********************************************************************/
STRING *make_seqn(const char *title, const char *cstring, int length)
{
    STRING *sptr;
    char *sequence;
    char buffer[CHAR_BUFFER_LEN];

    printf("make_seqn('%s', %d, %s)\n", title, length, get_char_array(cstring, length, buffer));
    
    if ((sequence = (char *)malloc(length)) == NULL) {
        fprintf(stderr, "Ran out of memory. Unable to add new sequence.\n");
        return NULL;
    }
    memcpy(sequence, cstring, length);

    if ((sptr = (STRING *)calloc(sizeof(STRING), 1)) == NULL) {
        free(sequence);
        return NULL;
    }
    
    sptr->sequence = sequence;
    sptr->length = length;
    strcpy(sptr->title, title);

    return sptr;
}

/********************************************************************** 
 *  Function: make_seq() by Peter                                           
 *                                                                    
 *  Parameter:                                                        
 *                                                                    
 * Convert a c string to a STRING
 * 
 **********************************************************************/
STRING *make_seq(const char *title, const char *cstring)
{
    STRING *sptr;
    char *sequence;

    printf("make_seq('%s', '%s')\n", title, cstring);
    
    if ((sequence = _strdup(cstring)) == NULL) {
        fprintf(stderr, "Ran out of memory. Unable to add new sequence.\n");
        return NULL;
    }
    if ((sptr = (STRING *)calloc(sizeof(STRING), 1)) == NULL) {
        free(sequence);
        return NULL;
    }
    
    sptr->sequence = sequence;
    sptr->length = strlen(cstring);
    strcpy(sptr->title, title);

    return sptr;
}

/********************************************************************
*  Function print_string
*
*  Parameters spt -- pointer to a STRING data type
*
*  Prints a strings information using the more interface
*  the output is sized to the screen and is displayed with
*  row and col labels. It is up to the calling function to call
*  mstart and mend.
*
*********************************************************************/
void print_string(STRING* string)
{
  int i, j, pos, width, height, space, pos_len, spcount;
  char *s;

  printf("IDENT: %s\n", string->ident);
  printf("TITLE: %s\n", string->title);
  printf("LENGTH: %d\n", string->length);
  printf("SEQUENCE:\n");

  width = 60;
  height = 30;
  pos_len = 10;

  s = string->sequence;
  pos = 0;
  while (pos < string->length) {
    /*
     * Print the header line
     */
    for (i = 0; i < pos_len; i++)
      mputc(' ');

    j = 1;
    for (i=0; i < width; i++) {
      for (space=0; space < 9; space++) {
        mputc(' ');
        i++;
      }
      printf("%1d", j++);
     
    }
    if (mputc('\n') == 0)
      return;

    /*
     * Print num_rows - 2 lines of the sequence.
     */
    for (i=0; pos < string->length && i < height; i++) {
      printf(" %7d  ", pos + 1);
      for (j=0,spcount=0; pos < string->length && j < width; j++) {
        if (!isprint((int)s[pos])) {
          mputc('#');
          pos++;
          if (s[pos-1] == '\n')
            break;
        }
        else
          mputc(s[pos++]);
    }

      if (mputc('\n') == 0)
        return;
    }
    mputs("\n\n");
  }
} 

void terse_print_string(STRING *spt)
{
  int i;
  char *s, *t, buffer[51];
  
//  printf("\tTYPE: %s\t", db_names[spt->db_type]);
  printf("TITLE: ");
  if (*(spt->title))
    mputs(spt->title);
  mputc('\n');

  printf("\tLENGTH: %d\t",spt->length);
 // printf("ALPHA: %s\n",alpha_names[spt->raw_alpha]);

  buffer[50] = '\0';
  printf("\tSEQUENCE:  ");
  for (s=buffer,t=spt->sequence,i=0; i < 50 && i < spt->length; i++,s++,t++)
    *s = (isprint((int)(*t)) ? *t : '#');
  *s = '\0';
  mputs(buffer);  
  if(spt->length > 50)
    mputs("...");
  mputc('\n');
}


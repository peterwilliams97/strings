#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "strmat.h"
#include "strmat_alpha.h"
#include "strmat_fileio.h"
#include "strmat_seqary.h"
#include "strmat_util.h"

//extern int util_menu(void);

extern FILE *fpout;

static int buf_len = 0;
static char *inbuf = NULL;

int select_sequences(char *, int);
int set_marks_from_list(char *);

/*********************************************************************
*  Function get_string
*  
*  Parameter: label -- either = "text" or "pattern" depending on 
*                      whether get_text or get_pattern was the
*                      calling function, respectively
*
* get_string will return a string from the user for the respective
* calling function.
* Depending on option that you choose you can either:
*       0)         read file
*       !)         List the current existing sequence..
*       num > 0)   To choose a current existing sequence.
*       *)         Go to the utilities menu         
*
*********************************************************************/
STRING *get_string(char *label) 
{
  int seq_index, num_seqs;
  STRING *s;

  while (1)  {
    num_seqs = get_num_sequences();
    if (num_seqs == 0) 
      printf("\nCommands (None");
    else
      printf("\nCommands (1-%d", num_seqs);
    printf(" - sequences, ! - list sequences, * - utilities menu,\n");
    printf("          r - read formatted file, t - create new sequence,");
    printf(" Ctl-D - cancel)\n");
    printf("Enter %s: ", label);

    if ((inbuf = my_getline(stdin, &buf_len)) == NULL) {
      printf("\n\n");
      return NULL;
    }

    switch (inbuf[0]) {
    case 't':
      type_in_seq();
      break;

    //case 'r':
    //  fread_formatted();
    //  break;

    //case '*':
    //  util_menu();
    //  break;

    case '!':
      list_sequences(3);
      break;

    default:
      if (sscanf(inbuf, "%d", &seq_index) == 1 &&
          (s = get_element(seq_index)) != NULL)
        return s;
      else
        printf("That is not a choice.  Please select from those shown\n");
      break;
    }
  }
}

/*********************************************************************
*  Function get_string_ary
*  
*  Parameter: label -- either = "text" or "pattern" depending on 
*                      whether get_text or get_pattern was the
*                      calling function, respectively
*
* get_string will return a string from the user for the respective
* calling function.
* Depending on option that you choose you can either:
*       0)         read file
*       *)         List the current existing sequence..
*       num > 0)   To choose a current existing sequence.
*       !)         Go to the utilities menu         
*
*********************************************************************/

STRING **get_string_ary(char *label, int *len_out)
{
  int  i, j, len, num_seqs;
  STRING **s;

  len = select_sequences(label, 1);
  if (len <= 0) {
    *len_out = 0;
    return NULL;
  }
  else {
    if ((s = malloc((len + 1) * sizeof(STRING *))) == NULL) {
      fprintf(stderr, "Ran out of memory.  Unable to select sequences.\n");
      *len_out = 0;
      return NULL;
    }

    num_seqs = get_num_sequences();
    for (i=1,j=0; i <= num_seqs; i++)
      if (get_mark(i) == 1)
        s[j++] = get_element(i);
    s[j] = NULL;

    *len_out = j;
    return s;
  }
}

/*********************************************************************
*  Function get_bounded
*  
*  Parameter: label -- label for the prompt
*             low   -- lower bound for the number to be read
*             high  -- upper bound for the number to be read
*             def   -- default number
*
* get_bounded will read a number from the use between the two bounds and
* return it to the calling function.
*
*********************************************************************/

int get_bounded(char *label, int low, int high, int def)
{
  int value;

  while (1) {
    printf("\nCommands (%d-%d - enter number, RET - default,"
           " Ctl-D - cancel)\n", low, high);

    printf("Enter %s [%d]: ", label, def);

    if ((inbuf = my_getline(stdin, &buf_len)) == NULL)
      return low - 1;
    else if (inbuf[0] == '\0')
      return def;

    if (sscanf(inbuf, "%d", &value) != 1)
      printf("\nThat is not a number."
             "  Please select from the choices shown.\n");
    else if (value < low || high < value)
      printf("\nThat number is out of range."
             "  Please select from the choices shown.\n");
    else 
      return value;
  }
}

/********************************************************************** 
 *  Function: list_sequences()
 *                                                                    
 *  Parameter:                                                        
 *     num_lines_after - The number of lines that will be printed
 *                       after this functions output is printed.
 *                                                                    
 *  This function prints information about all of the sequences
 *  currently in memory.
 *
 * 
 **********************************************************************/
void list_sequences(int num_lines_after)
{
  int i, num_seqs, status;
  STRING *spt;

  num_seqs = get_num_sequences();
  if (num_seqs == 0) {
    printf("\nThere are no sequences to list.\n");
    return;
  }

  mstart(stdin, stdout, OK, OK, 5, NULL);
  status = 1;
  for (i=1; i <= num_seqs && status; i++)  {
    spt = get_element(i);

   // mprintf("\n%d)\tTYPE: %s\tTITLE: %s\n", i, db_names[spt->db_type], spt->title);
    //status = mprintf("\tLENGTH: %d\tALPHA: %s\n",  spt->length, alpha_names[spt->raw_alpha]);
  }
  mputc('\n');
  mend(num_lines_after);
}
  
/********************************************************************** 
 *  Function: print_seq()
 *                                                                    
 *  Parameter:                                                        
 *     num_lines_after - The number of lines that will be printed
 *                       after this functions output is printed.
 *                                                                    
 *  This function prints detailed information about one of the sequences
 *  currently in memory.
 *
 * 
 **********************************************************************/
void print_seq(int num_lines_after)
{
  STRING *spt;

  if (get_num_sequences() == 0) {
    printf("\nThere are no sequences to print.\n");
    return;
  }

  if ((spt = get_string("sequence to print")))  {
    mstart(stdin, fpout, OK, OK, 5, NULL);
    mputs("\n\n");
    print_string(spt);
    mend(num_lines_after);
  }
}

/********************************************************************** 
 *  Function: type_in_seq()                                           
 *                                                                    
 *  Parameter:                                                        
 *                                                                    
 *  This function is in charge of storing in a pattern typed in       
 *  from the keyboard into memory (known_seq).  The maximum number    
 *  of is limited by memory. 
 *
 * 
 **********************************************************************/
void type_in_seq(void)
{
  int i, elem_num, seqlen, seqsize, alpha, orig_seqlen, otherflag;
  char ch, newline_ch, *s, *sequence, *seq2, title[TITLE_LENGTH+1];
  STRING *sptr;

  printf("\nEnter sequence title (Ctl-D to cancel): ");
  if ((inbuf = my_getline(stdin, &buf_len)) == NULL) {
    printf("\n\n");
    return;
  }
  title[TITLE_LENGTH] = '\0';
  if (*inbuf != '\0')
    strncpy(title, inbuf, TITLE_LENGTH);

  alpha = -1;
  while (alpha == -1) {
    printf("Alphabets (1 - DNA, 2 - RNA, 3 - PROTEIN, 4 - ASCII, Ctl-D - Cancel)\n");
    printf("Enter alphabet: ");

    if ((inbuf = my_getline(stdin, &buf_len)) == NULL) {
      printf("\n\n");
      return;
    }

    switch(*inbuf)  {
    case '1':
      alpha = ALPHA_DNA;
      break;

    case '2':
      alpha = ALPHA_RNA;
      break;

    case '3':
      alpha = ALPHA_PROTEIN;
      break;

    case '4':
      alpha = ALPHA_ASCII;
      break;

    default:
      printf("\nThat is not a choice.\n");
    }
  }
  newline_ch = rawmapchar(alpha, '\n');
  if (newline_ch == -1)
    return;

  seqsize = 256;
  if ((sequence = malloc(seqsize)) == NULL) {
    fprintf(stderr, "Ran out of memory. Unable to add new sequence.\n");
    return;
  }

  printf("\nEnter sequence (To end sequence, type Ctl-D at the beginning of a line):\n");

  seqlen = 0;
  while ((inbuf = my_getline(stdin, &buf_len)) != NULL) {
    if (seqlen + buf_len >= seqsize) {
      seqsize += seqlen + buf_len;
      if ((sequence = realloc(sequence, seqsize)) == NULL) {
        fprintf(stderr, "Ran out of memory. Unable to add new sequence.\n");
        return;
      }
    }

    orig_seqlen = seqlen;
    otherflag = 0;
    for (i=0,s=inbuf; i < buf_len; i++,s++) {
      ch = rawmapchar(alpha, *s);
      if (ch == -1)
        return;
      else if (ch != '\0')
        sequence[seqlen++] = ch;
      else if (!isspace((int)(*s)))
        otherflag = 1;
    }
    if (newline_ch != '\0')
      sequence[seqlen++] = newline_ch;

    if (otherflag) {
      printf("\nError:  This line contains characters not in the alphabet.\n");
      while (1) {
        printf("Remove this line from the sequence (default: y)? ");
        while ((inbuf = my_getline(stdin, &buf_len)) == NULL) ;

        if (toupper(inbuf[0]) == 'Y' || inbuf[0] == '\0') {
          seqlen = orig_seqlen;
          break;
        }
        else if (toupper(inbuf[0]) == 'N')
          break;

        printf("Answer y or n please.\n");
      }
      putchar('\n');
    }   
  }
  sequence[seqlen] = '\0';
  if (seqlen > 0 && sequence[seqlen-1] == newline_ch)
    sequence[--seqlen] = '\0';

  if (seqlen == 0) {
    printf("\nNo sequence entered.  Did not create a sequence structure.\n");
    free(sequence);
    return;
  }

  ;
  if ((seq2 = malloc(seqlen + 1)) == NULL) {
    fprintf(stderr, "Ran out of memory.  Unable to add new sequence.\n");
    free(sequence);
    return;
  }

  if ((elem_num = add_element()) == -1) {
    free(sequence);
    free(seq2);
    return;
  }
    
  sptr = get_element(elem_num);
  sptr->db_type = 0;
  sptr->raw_alpha = alpha;
  sptr->raw_seq = sequence;
  sptr->sequence = seq2;
  sptr->length = seqlen;
  strcpy(sptr->title, title);

  printf("\n\n");
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
    int i, seqlen, seqsize, alpha, orig_seqlen, otherflag;
    char ch, newline_ch, *s, *sequence, *seq2;
    STRING *sptr;

    printf("make_seq('%s', '%s')\n", title, cstring);

    inbuf = (char *)cstring;
    buf_len = strlen(cstring);
    alpha = ALPHA_ASCII;
    newline_ch = rawmapchar(alpha, '\n');
    if (newline_ch == -1) {
        return 0;
    }

    seqsize = 256;
    if ((sequence = calloc(seqsize,1)) == NULL) {
        fprintf(stderr, "Ran out of memory. Unable to add new sequence.\n");
        return 0;
    }

    seqlen = 0;
    orig_seqlen = seqlen;
    otherflag = 0;
    for (i=0,s=inbuf; i < buf_len; i++,s++) {
        ch = rawmapchar(alpha, *s);
        if (ch == -1) {
            return 0;
        } else if (ch != '\0') {
            sequence[seqlen++] = ch;
        } else if (!isspace((int)(*s))) {
            printf("Bad char '%c' (%d) => '%c'\n", *s, (unsigned int)*s, ch);
            otherflag = 1;
        }
    }
    if (newline_ch != '\0') {
      sequence[seqlen++] = newline_ch;
    }

    if (otherflag) {
        printf("\nError:  This line contains characters not in the alphabet.\n");
    }   
 
    sequence[seqlen] = '\0';
    if (seqlen > 0 && sequence[seqlen-1] == newline_ch)
        sequence[--seqlen] = '\0';

    if ((seq2 = calloc(seqlen + 1, 1)) == NULL) {
        fprintf(stderr, "Ran out of memory.  Unable to add new sequence.\n");
        free(sequence);
        return 0;
    }
    strcpy(seq2, cstring);
           
    if ((sptr = malloc(sizeof(STRING))) == NULL)
        return NULL;
    memset(sptr, 0, sizeof(STRING));
    sptr->db_type = 0;
    sptr->raw_alpha = alpha;
    sptr->alpha_size = get_alpha_size(alpha);
    sptr->raw_seq = seq2;
    sptr->sequence = sequence;
    sptr->length = seqlen;
    strcpy(sptr->title, title);

    return sptr;
}

/**************************************************************
* Function delete_seq()
*
* parameters:  none
*
*
* This function allows the user to delete selected sequences from
* the current list in memory.
*
****************************************************************/

void delete_seq(void)
{
  int i, num_seqs;

  num_seqs = get_num_sequences();
  if (num_seqs == 0) {
    printf("\nThere are no sequences to delete.\n");
    return;
  }

  if (select_sequences("sequences to delete", 0) > 0) {
    for (i=num_seqs; i > 0; i--)
      if (get_mark(i) == 1)
        delete_element(i);
  }
}
 
/*
 *  Function get_seq_numbers 
 *
 *  Parameters label -- contains "save" or "delete"
 *                      depending on calling function
 *
 *  This function returns an array of sequence number
 *  or null if the user wants to return to previous
 *  menu.
 *
 */
int select_sequences(char* label, int mode)
{
  int len, num_seqs, flag;

  flag = 1;
  while (1) {
    if (flag) {
      printf("\nList sequences separated by spaces, "
             "or for a range separated by '-'.\n");
      printf("Note that for a range the second number "
             "must be greater than the first.\n");
      printf("All duplicate entries are ignored.\n");
      printf("Example:  1 3-5 7 9-10\n");
      flag = 0;
    }

    num_seqs = get_num_sequences();
    if (num_seqs == 0) 
      printf("\nCommands (None");
    else
      printf("\nCommands (1-%d", num_seqs);
    printf(" - sequences, ! - list sequences, * - utilities menu,\n");
    if (mode == 1)
      printf("          r - read formatted file, t - create new sequence,\n");
    printf("          ? - help, Ctl-D - cancel)\n");
    printf("Enter %s: ", label);

    if ((inbuf = my_getline(stdin, &buf_len)) == NULL) {
      printf("\n\n");
      return 0;
    }

    switch(inbuf[0]) {
    case 't':
      if (mode == 1)
        type_in_seq();
      else
        printf("\nThat is not a valid choice."
               "  Please select from those shown.\n");
      break;

    //case 'r':
    //  if (mode == 1)
    //    fread_formatted();
    //  else
    //    printf("\nThat is not a valid choice."
    //           "  Please select from those shown.\n");
    //  break;

    //case '*':
    //  util_menu();
    //  break;

    case '!':
      list_sequences((mode == 1 ? 4 : 3));
      break;

      
    case '?':
      printf("\nList sequences separated by spaces, "
             "or for a range separated by '-'.\n");
      printf("Note that for a range the second number "
             "must be greater than the first.\n");
      printf("All duplicate entries are ignored.\n");
      printf("Example:  1 3-5 7 9-10\n");
      break;
      
    default:
      if (num_seqs == 0)
        printf("\nThere are no sequences to select.\n");
      else {
        len = set_marks_from_list(inbuf);
        if (len > 0)
          return len;
      }
    }
  }
}

int set_marks_from_list(char *s)
{
  int i, len, num1, num2, num_seqs;
  
  num_seqs = get_num_sequences();
  for (i=0; i < num_seqs; i++)
    set_mark(i, 0);

  len = 0;
  while (*s) {
    while (*s && isspace((int)(*s))) s++;
    if (!*s)
      break;

    if (sscanf(s, "%d", &num1) == 0) {
      printf("\nThat is not a valid list of sequences."
             "  Type ? for the correct format.\n");
      return -1;
    }
    else if (num1 <= 0 || num1 > num_seqs) {
      printf("\n%d is out of the range of current sequences.\n", num1);
      printf("Please select from the range of 1 - %d.\n", num_seqs);
      return -1;
    }
    while (*s && isdigit((int)(*s))) s++;

    if (!*s || isspace((int)(*s))) {
      set_mark(num1, 1);
      len++;
    }
    else if (*s == '-') {
      if (sscanf(++s, "%d", &num2) == 0) {
        printf("\nThat is not a valid list of sequences."
               "  Type ? for the correct format.\n");
        return -1;
      }
      else if (num2 <= 0 || num2 > num_seqs) {
        printf("%d is out of the range of current sequences.\n", num2);
        printf("Please select from the range of 1 - %d.\n", num_seqs);
        return -1;
      }

      if (num1 > num2) {
        printf("\n%d-%d is an invalid range.\n", num1, num2);
        printf("A range's first number must be less than the second.\n");
        return -1;
      }
      
      for (i=num1; i <= num2; i++) {
        set_mark(i, 1);
        len++;
      }
      while (*s && isdigit((int)(*s))) s++;
    }
    else {
      printf("\nThat is not a valid list of sequences."
             "  Type ? for the correct format.\n");
      return -1;
    }
  }  

  return len;    
}

/*
 *  FUNCTION fread_formatted()
 *
 *  Parameters:  none
 *
 *  This function allows the user to specify an input file
 *  and sequentially reads that file for all sequences contained.
 *  If errors are found in either file format, sequence length
 *  or sequence alphabet this function prompts the user
 *  with a continue to read even though errors where found.
 *
 */
//void fread_formatted(void)
//{
//  int elem_num, status;
//  char *filename;
//  FILE *fp;
//  STRING *new_string;  
//
//  do {
//    printf("\nEnter file name (Ctl-D to cancel): ");
//    if ((inbuf = my_getline(stdin, &buf_len)) == NULL) {
//      printf("\n\n");
//      return;
//    }
//  } while(*inbuf == '\0');
//
//  if ((filename = malloc(buf_len + 1)) == NULL)  {
//    fprintf(stderr,
//            "\nRan out of memory.  Unable to read specified file.\n\n");
//    return;
//  }
//  strcpy(filename, inbuf);
//
//  printf("\nOpening file %s for input...", filename);
//  fflush(stdout);
//  if ((fp = fopen(filename, "r")) == NULL)  {
//    fprintf(stderr, "\nError: Unable to open file %s.\n\n", filename);
//    free(filename);
//    return;
//  } 
//
//  new_string = NULL;
//  while (1) {
//    status = find_next_sequence(fp);
//    if (status == ERROR)
//      break;
//    else if (status == FORMAT_ERROR) {
//      fprintf(stderr, "\nError: Format error found while reading file %s.\n\n",
//              filename);
//      free(filename);
//      fclose(fp);
//      return;
//    }
//
//    printf("\nReading sequence...");
//    fflush(stdout);
//    if ((elem_num = add_element()) == 0)  {
//      fprintf(stderr,
//              "\nError: could not allocate space for new element.\n\n");
//      free(filename);
//      fclose(fp);
//      return;
//    }
//    new_string = get_element(elem_num);
//
//    switch (read_sequence(fp, new_string)) {
//    case OK:
//      if (new_string->title[0] != '\0')
//        printf("%s...", new_string->title);
//      printf("done.");
//      break;
//
//    case (int)NULL:         // !@#$
//      delete_element(elem_num);
//      break;
//
//    case PREMATURE_EOF:
//      printf("\nError: file %s ended prematurely.\n", filename);
//      delete_element(elem_num);
//      break;
//
//    case FORMAT_ERROR:
//      if (new_string->length == 0)
//        fprintf(stderr, "\nError: header for sequence incomplete.\n");
//      else
//        fprintf(stderr, "\nError: format error found in sequence %s.\n",
//               new_string->title);
//      delete_element(elem_num);
//      break;
//
//    case MISMATCH:
//      printf("\nWarning: found characters in %s that do not match alphabet.",
//             new_string->title);
//    }
//  }
//  printf("\nClosing %s.\n\n", filename);
//  fclose(fp);
//  free(filename);
//}

/**************************************************************************
 *
 *  FUNCTION fwrite_formatted()
 *
 *  Parameters:  none
 *
 *  This function allows the user to specify an output file
 *  and sequentially write out the specified sequences
 *  that file for all sequences contained.
 *  If errors are found in either file format, sequence length
 *  or sequence alphabet this function prompts the user
 *  with a continue to read even though errors where found.
 *
 ***************************************************************************/
//void fwrite_formatted(void)
//{
//  int i, len, num_seqs;
//  char *filename;
//  STRING *new_string;
//  FILE* fp;
//
//  do {
//    printf("\nEnter file name (Ctl-D to cancel): ");
//    if((inbuf = my_getline(stdin, &buf_len)) == NULL) {
//      printf("\n\n");
//      return;
//    }
//  } while(*inbuf == '\0');
//
//  if ((filename = malloc(buf_len + 1)) == NULL)  {
//    fprintf(stderr, "\nError: malloc failed in fwrite_formatted().\n");
//    return;
//  }
//  strcpy(filename, inbuf);
//
//  len = select_sequences("sequences to save to file", 0);
//  if (len <= 0) {
//    free(filename);
//    return;
//  }
//  putchar ('\n');
//
//  printf("\nOpening file %s for output...", filename);
//  if ((fp = fopen(filename, "w")) == NULL)   {
//    printf("\nError: could not open file %s for output.\n", filename);
//    free(filename);
//    return;
//  }
//  putchar('\n');
//
//  num_seqs = get_num_sequences();
//  for (i=1; i <= num_seqs; i++) {
//    if (get_mark(i) == 1) {
//      new_string = get_element(i);
//      printf("Writing sequence %s...", new_string->title);
//      fflush(stdout);
//      switch (write_sequence(fp, new_string)) {
//      case OK:
//        printf("done.\n");
//        break;
//
//      case FORMAT_ERROR:
//        fprintf(stderr, "\nError: sequence format for %s is incorrect.\n",
//                new_string->title);
//        break;
//
//      case (int)NULL:   // !@#$
//      case MISMATCH:
//        break;
//      }
//    }
//  }
//
//  printf("Closing %s.\n", filename);
//  fclose(fp);
//  free(filename);
//}

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
  int ascii;
  char *s;

  //mprintf("TYPE: %s\n", db_names[string->db_type]);
  mprintf("IDENT: %s\n", string->ident);
  mprintf("TITLE: %s\n", string->title);
 // mprintf("ALPHA: %s\n", alpha_names[string->raw_alpha]); 
  mprintf("LENGTH: %d\n", string->length);
  mprintf("SEQUENCE:\n");

  ascii = (string->raw_alpha == ALPHA_ASCII);

  width = 60;
  height = 30;
  pos_len = 10;

  s = string->raw_seq;
  pos = 0;
  while (pos < string->length) {
    /*
     * Print the header line
     */
    for (i=0; i < pos_len; i++)
      mputc(' ');

    j = 1;
    for (i=0; i < width; i++) {
      for (space=0; space < 9; space++) {
        mputc(' ');
        i++;
      }
      mprintf("%1d", j++);
      if (!ascii)
        mputc(' ');
    }
    if (mputc('\n') == 0)
      return;

    /*
     * Print num_rows - 2 lines of the sequence.
     */
    for (i=0; pos < string->length && i < height; i++) {
      mprintf(" %7d  ", pos + 1);
      for (j=0,spcount=0; pos < string->length && j < width; j++) {
        if (!isprint((int)s[pos])) {
          mputc('#');
          pos++;
          if (s[pos-1] == '\n')
            break;
        }
        else
          mputc(s[pos++]);

        if (!ascii && ++spcount == 10) {
          mputc(' ');
          spcount = 0;
        }
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
  
//  mprintf("\tTYPE: %s\t", db_names[spt->db_type]);
  mprintf("TITLE: ");
  if (*(spt->title))
    mputs(spt->title);
  mputc('\n');

  mprintf("\tLENGTH: %d\t",spt->length);
 // mprintf("ALPHA: %s\n",alpha_names[spt->raw_alpha]);

  buffer[50] = '\0';
  mprintf("\tSEQUENCE:  ");
  for (s=buffer,t=spt->raw_seq,i=0; i < 50 && i < spt->length; i++,s++,t++)
    *s = (isprint((int)(*t)) ? *t : '#');
  *s = '\0';
  mputs(buffer);  
  if(spt->length > 50)
    mputs("...");
  mputc('\n');
}

/***************************************************************
*  FUNCTION fread_unformatted()
*
*  Parameters:  none
*
*  This function allows the user to specify an input file
*  and sequentially reads that file for all sequences contained.
*  If errors are found in either file format, sequence length
*  or sequence alphabet this function prompts the user
*  with a continue to read even though errors where found.
*
****************************************************************/
void fread_unformatted(void)
{
  int elem_num, other_chars, seqlen, seqsize, alpha;
  char ch, mapch, *filename, *sequence, *seq2, title[TITLE_LENGTH+1];
  STRING *sptr;  
  FILE* fp;

  printf("\nEnter sequence title (Ctl-D to cancel): ");
  if ((inbuf = my_getline(stdin, &buf_len)) == NULL) {
    printf("\n\n");
    return;
  }
  title[0] = title[TITLE_LENGTH] = '\0';
  if (*inbuf != '\0')
    strncpy(title, inbuf, TITLE_LENGTH);

  alpha = -1;
  while (alpha == -1) {
    printf("Alphabets "
           "(1 - DNA, 2 - RNA, 3 - PROTEIN, 4 - ASCII, Ctl-D - Cancel)\n");
    printf("Enter alphabet: ");

    if ((inbuf = my_getline(stdin, &buf_len)) == NULL) {
      printf("\n\n");
      return;
    }

    switch(*inbuf)  {
    case '1':
      alpha = ALPHA_DNA;
      break;

    case '2':
      alpha = ALPHA_RNA;
      break;

    case '3':
      alpha = ALPHA_PROTEIN;
      break;

    case '4':
      alpha = ALPHA_ASCII;
      break;

    default:
      printf("\nThat is not a choice.\n");
    }
  }

  do  {
    printf("\nEnter file name (Ctl-D to cancel): ");
    if((inbuf = my_getline(stdin, &buf_len)) == NULL) {
      printf("\n\n");
      return;
    }
  } while (*inbuf == '\0');

  if ((filename = malloc(buf_len + 1)) == NULL)  {
    fprintf(stderr, "\nError: malloc failed in fread_unformatted().\n");
    return;
  }
  strcpy(filename, inbuf);

  printf("\nOpening file %s for input...\n", filename);
  if ((fp = fopen(filename, "r")) == NULL)  {
    fprintf(stderr, "Error: could not open file %s for input.\n", filename);
    free(filename);
    return;
  }
  
  seqsize = 256;
  if ((sequence = malloc(seqsize)) == NULL) {
    fprintf(stderr, "Ran out of memory. Unable to add new sequence.\n");
    free(filename);
    return;
  }

  seqlen = 0;
  other_chars = 0;
  while ((ch = getc(fp)) != EOF) {
    mapch = rawmapchar(alpha, ch);
    if (mapch == -1) {
      free(filename);
      free(sequence);
      return;
    }
    else if (mapch != '\0') {
      if (seqlen == seqsize) {
        seqsize += seqsize;
        if ((sequence = realloc(sequence, seqsize)) == NULL) {
          fprintf(stderr,
                  "Call to realloc failed.  Unable to add the sequence.\n");
          free(filename);
          free(sequence);
          return;
        }
      }
      sequence[seqlen++] = mapch;
    }
    else if (!isspace((int)ch))
      other_chars = 1;
  }
  fclose(fp);
  free(filename);

  if (other_chars) {
    printf("\nError:  This file contains characters not in the alphabet.\n");
    while (1) {
      printf("Abort the new sequence creation (default: y)?");
      while ((inbuf = my_getline(stdin, &buf_len)) == NULL) ;

      if (toupper(inbuf[0]) == 'Y' || inbuf[0] == '\0') {
        free(filename);
        free(sequence);
        return;
      }
      else if (toupper(inbuf[0]) == 'N')
        break;

      printf("Answer y or n please.\n");
    }
  }    

  sequence[seqlen] = '\0';
  if (seqlen == 0) {
    printf("\nNo sequence entered.  Did not create a sequence structure.\n");
    free(sequence);
    return;
  }

  if ((seq2 = malloc(seqlen + 1)) == NULL) {
    fprintf(stderr, "Ran out of memory.  Unable to add new sequence.\n");
    free(sequence);
    return;
  }

  if ((elem_num = add_element()) == -1) {
    free(sequence);
    free(seq2);
    return;
  }
    
  sptr = get_element(elem_num);
  sptr->db_type = 0;
  sptr->raw_alpha = alpha;
  sptr->raw_seq = sequence;
  sptr->sequence = seq2;
  sptr->length = seqlen;
  strcpy(sptr->title, title);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strmat.h"
#include "strmat_seqary.h"

#define INIT_SEQ_ARY_SIZE 40

static STRING **sequences = NULL;
static int *marks = NULL;
static int num_seqs, seqsize;

STRING *init_string();
void free_string(STRING *);

void create_seq_array()
{
  int i;

  seqsize = INIT_SEQ_ARY_SIZE;
  num_seqs = 0;

  if ((sequences = malloc(seqsize * sizeof(STRING *))) == NULL) {
    fprintf(stderr, "malloc failed while initializing array of sequences.\n");
    exit(1);
  }

  if ((marks = malloc(seqsize * sizeof(int))) == NULL) {
    fprintf(stderr, "malloc failed while initializing array of sequences.\n");
    exit(1);
  }

  for (i=0; i < seqsize; i++) {
    sequences[i] = NULL;
    marks[i] = 0;
  }
}

STRING *get_element(int num)
{
  if (num >= 1 && num <= num_seqs)
    return sequences[num-1];
  else
    return NULL;
}

int get_num_sequences()
{
  return num_seqs;
}

void set_mark(int num, int value)
{
   if (num >= 1 && num <= num_seqs)
    marks[num-1] = value;
} 

int get_mark(int num)
{
  if (num >= 1 && num <= num_seqs)
    return marks[num-1];
  else
    return 0;
} 

int delete_element(int element)
{
  int i;

  if (element < 1 || element > num_seqs)
    return -1;

  free_string(sequences[element-1]);
  for (i=element-1; i < num_seqs - 1; i++)  {
    sequences[i] = sequences[i+1];
    marks[i] = marks[i+1];
  }
  sequences[i] = NULL;
  marks[i] = 0;
  num_seqs--;

  return 0;
}

int add_element(void)
{
  int i;

  if (num_seqs == seqsize) {
    seqsize += seqsize;
    if ((sequences = realloc(sequences, seqsize * sizeof(STRING *))) == NULL) {
      fprintf(stderr, "\nRan out of memory.  Cannot add new sequence.\n");
      return -1;
    }
    if ((marks = realloc(marks, seqsize * sizeof(int))) == NULL) {
      fprintf(stderr, "\nRan out of memory.  Cannot add new sequence.\n");
      return -1;
    }

    for (i=num_seqs; i < seqsize; i++) {
      sequences[i] = NULL;
      marks[i] = 0;
    }
  }

  sequences[num_seqs] = init_string();
  marks[num_seqs] = 0;
  num_seqs++;

  return num_seqs;
}

void free_seq_ary()
{
 int i;

 for (i=num_seqs; i > 0; i--)
   delete_element(i);
 free(sequences);
 free(marks);
}

STRING *init_string()
{
  STRING *sptr;

  if ((sptr = malloc(sizeof(STRING))) == NULL)
    return NULL;

  memset(sptr, 0, sizeof(STRING));

  return sptr;
}

void free_string(STRING *sptr)
{
  if (sptr == NULL)
    return;

  if (sptr->sequence != NULL)  free(sptr->sequence);
  if (sptr->desc_string != NULL)  free(sptr->desc_string);
  if (sptr->raw_seq != NULL)  free(sptr->raw_seq);

  free(sptr);
}

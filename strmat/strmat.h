#ifndef _STRMAT_H_
#define _STRMAT_H_

#include "peter_common.h"

/*
 * STRING data structure.
 *
 * Contains the sequence and data structures specifically associated with
 * that string.
 *     sequence    - the  characters of the string
 *     length      - the length of the string
 *     ident       - an database identifier for retrieving the string
 *     title       - a title or name for the string
 */

#define IDENT_LENGTH 50
#define TITLE_LENGTH 200

struct STRING {
    CHAR_TYPE *sequence;
    int length;
    char ident[IDENT_LENGTH+1], title[TITLE_LENGTH+1];
};



#endif

#ifndef _STRMAT_PRINT_H_
#define _STRMAT_PRINT_H_

#include "stree_strmat.h"

void print_node(const STREE_NODE node, const char *title = 0);
int large_print_tree(SUFFIX_TREE, STREE_NODE, int);
void small_print_tree(SUFFIX_TREE, STREE_NODE, int, int);

#endif

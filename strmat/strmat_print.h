#ifndef _STRMAT_PRINT_H_
#define _STRMAT_PRINT_H_

#include <string>
#include "stree_strmat.h"

void print_node(const STREE_NODE node, const char *title = 0);
void print_label(const SUFFIX_TREE tree, const STREE_NODE z, std::string title);
int large_print_tree(SUFFIX_TREE, STREE_NODE, int);
void small_print_tree(SUFFIX_TREE, STREE_NODE, int, int);

#endif


#ifndef _LCA_H_
#define _LCA_H_

typedef enum { LCA_NAIVE, LCA_LINEAR, LCA_NLOGN } LCA_TYPE;

typedef struct {
  LCA_TYPE type;
  SUFFIX_TREE tree;

  unsigned int *I, *A;
  STREE_NODE *L;

  int num_prep, num_compares;
} LCA_STRUCT;

LCA_STRUCT *lca_prep(SUFFIX_TREE tree);
STREE_NODE lca_lookup(LCA_STRUCT *lca, STREE_NODE x, STREE_NODE y);
void lca_free(LCA_STRUCT *lca);
LCA_STRUCT *lca_naive_prep(SUFFIX_TREE tree);
STREE_NODE lca_naive_lookup(LCA_STRUCT *lca, STREE_NODE x, STREE_NODE y);
void lca_naive_free(LCA_STRUCT *lca);

#endif

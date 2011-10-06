/*
 * stree_lca.c
 *
 * Implementations of several least common ancestor algorithms for
 * suffix trees.
 *
 * NOTES:
 *    5/96  -  Original Implementation of linear time algorithm
 *             (Bill Coffman)
 *    7/96  -  Modularized that code, and added the naive algorithm
 *             (James Knight)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef STRMAT
#include "stree_strmat.h"
#else
#include "stree.h"
#endif
#include "stree_lca.h"


/*
 *
 * Forward References.
 *
 */
static int compute_I_and_L(LCA_STRUCT *lca, SUFFIX_TREE tree, STREE_NODE node);
static void compute_A(LCA_STRUCT *lca, SUFFIX_TREE tree, STREE_NODE node,
                      unsigned int Amask);
static unsigned int h(unsigned int number);
static unsigned int MSB(unsigned int number);



/*
 *
 *
 * Functions handling the finding of the least and most significant
 * bits of a number (used by the LCA algorithm).
 *
 * The functions are not completely general, as given in the book, but
 * assume that the numbers are 32 bits or less, and are implemented
 * using an 8 bit table and checking successive bytes of each number.
 */

static unsigned int msb_table[256], lsb_table[256];
static int init_flag = 0;

/*
 * init_tables
 *
 * Initialize the tables so that the msb_table and lsb_table contain
 * the most and least significant bit positions for the numbers 1..255.
 *
 * Parameters:  none
 *
 * Returns:  nothing
 */
static void init_tables(void)
{
  int i, j, lsb, msb, mask;

  j = 1;
  msb = -1;
  for (i=1; i < 256; i++) {
    if (i == j) {
      j <<= 1;
      msb++;
    }
    msb_table[i] = msb;

    for (lsb=0,mask=1; !(mask & i); lsb++,mask<<=1) ;
    lsb_table[i] = lsb;
  }

  init_flag = 1;
}


/*
 * h
 *
 * Return the position of the least significant 1 bit in the number.
 *
 * Parameters:  number  -  a number
 * 
 * Returns:  the position of the least significant 1 bit
 */
static unsigned int h(unsigned int number)
{
  if (!init_flag)
    init_tables();

  if (number & 0xff)
    return lsb_table[number & 0xff];
  number >>= 8;
  if (number & 0xff)
    return 8 + lsb_table[number & 0xff];
  number >>= 8;
  if (number & 0xff)
    return 16 + lsb_table[number & 0xff];
  number >>= 8;
  if (number & 0xff)
    return 24 + lsb_table[number & 0xff];

  return 32;
}

/*
 * MSB
 *
 * Return the position of the most significant 1 bit in the number.
 *
 * Parameters:  number  -  a number
 * 
 * Returns:  the position of the most significant 1 bit
 */
static unsigned int MSB(unsigned int number)
{
  if (!init_flag)
    init_tables();

  if (number & (0xff << 24))
    return 24 + msb_table[number >> 24];
  if (number & (0xff << 16))
    return 16 + msb_table[number >> 16];
  if (number & (0xff << 8))
    return 8 + msb_table[number >> 8];
  if (number & 0xff)
    return msb_table[number];

  return 32;
}
  


/*
 *
 *
 * The Constant Time LCA Algorithm
 *
 *
 */


/*
 * lca_prep
 *
 * Preprocessing for the constant time LCA algorithm.
 *
 * Parameters:  tree  -   a suffix tree
 *
 * Returns:  An LCA_STRUCT structure
 */
LCA_STRUCT *lca_prep(SUFFIX_TREE tree)
{
  int num_nodes;
  LCA_STRUCT *lca;

  if (tree == NULL)
    return NULL;

  if ((lca = malloc(sizeof(LCA_STRUCT))) == NULL)
    return NULL;

  memset(lca, 0, sizeof(LCA_STRUCT));
  lca->type = LCA_LINEAR;
  lca->tree = tree;

  num_nodes = stree_get_num_nodes(tree) + 1;

  if ((lca->I = malloc(num_nodes * sizeof(unsigned int))) == NULL) {
    lca_free(lca);
    return NULL;
  }
  memset(lca->I, 0, num_nodes * sizeof(unsigned int));

  if ((lca->A = malloc(num_nodes * sizeof(unsigned int))) == NULL) {
    lca_free(lca);
    return NULL;
  }
  memset(lca->A, 0, num_nodes * sizeof(unsigned int));

  if ((lca->L = malloc(num_nodes * sizeof(STREE_NODE))) == NULL) {
    lca_free(lca);
    return NULL;
  }
  memset(lca->L, 0, num_nodes * sizeof(STREE_NODE));

  /*
   * Compute the I and L values, then compute the A values.
   */
  compute_I_and_L(lca, tree, stree_get_root(tree));
  compute_A(lca, tree, stree_get_root(tree), 0);
    
  return lca;
}

/*
 * compute_I_and_L
 *
 * Compute the I values and L values for the LCA preprocessing.  The I
 * values are, for each node, the identifier with the largest least
 * significant 1 bit in the subtree rooted at the node.  The L values are,
 * for each node corresponding to an I value, the node at the head of each
 * "run" in the tree.
 *
 * Parameters:  lca   -  an LCA_STRUCT structure
 *              tree  -  a suffix tree
 *              node  -  a suffix tree node
 *
 * Returns:  the identifier with the largest least significant 1 bit in
 *           the subtree rooted at node.
 */
static int compute_I_and_L(LCA_STRUCT *lca, SUFFIX_TREE tree, STREE_NODE node)
{
  unsigned int id, Ival, Imax;
  STREE_NODE child;

  /*
   * Shift idents so that they go from 1..num_nodes.
   */
  id = (unsigned int) stree_get_ident(tree, node) + 1;

  /*
   * Find the node with the maximum I value in the subtree.
   */
  Imax = id;
  child = stree_get_children(tree, node);
  while (child != NULL) {
    Ival = compute_I_and_L(lca, tree, child);
    if (h(Ival) > h(Imax))
      Imax = Ival;

#ifdef STATS
    lca->num_prep++;
#endif

    child = stree_get_next(tree, child);
  }

  lca->I[id] = Imax;
  lca->L[Imax] = node;    /* will be overwritten by the highest node in run */

  return Imax;
}


/*
 * compute_A
 *
 * Computes the A values for the LCA preprocessing.  The A values are, for
 * each node, the heights of the least significant bits of the ancestors
 * of the node (where the bits of each A value are set to 1 for each such
 * height of an ancestor).
 *
 * Parameters:  lca    -  an LCA_STRUCT structure
 *              tree   -  a suffix tree
 *              node   -  a suffix tree node
 *              Amask  -  the bits set by the ancestors of node
 *
 * Returns:  nothing
 */
static void compute_A(LCA_STRUCT *lca, SUFFIX_TREE tree, STREE_NODE node,
                      unsigned int Amask)
{
  unsigned int id;
  STREE_NODE child;

  /*
   * Shift idents so that they go from 1..num_nodes.
   */
  id = (unsigned int) stree_get_ident(tree, node) + 1;

  Amask |= 1 << h(lca->I[id]);
  lca->A[id] = Amask;

  child = stree_get_children(tree, node);
  while (child != NULL) {
    compute_A(lca, tree, child, Amask);

#ifdef STATS
    lca->num_prep++;
#endif

    child = stree_get_next(tree, child);
  }
}


/*
 * lca_lookup
 *
 * Perform the constant time LCA computation, finding the least
 * common ancestor of x and y.
 *
 * Parameters:  lca  -  an LCA_STRUCT structure
 *              x    -  a suffix tree node
 *              y    -  another suffix tree node
 *
 * Returns:  the suffix tree node which is the LCA of x and y
 */
STREE_NODE lca_lookup(LCA_STRUCT *lca, STREE_NODE x, STREE_NODE y)
{
  unsigned int xid, yid, k, b, j, l, Iw, mask, *I, *A;
  STREE_NODE *L, w, xbar, ybar;
  SUFFIX_TREE tree;

  if (lca == NULL || lca->type != LCA_LINEAR || x == NULL || y == NULL)
    return NULL;
  
  tree = lca->tree;
  I = lca->I;
  A = lca->A;
  L = lca->L;

  /*
   * Shift idents so that they go from 1..num_nodes.
   */
  xid = (unsigned int) stree_get_ident(tree, x) + 1;
  yid = (unsigned int) stree_get_ident(tree, y) + 1;

  /*
   * Steps 1 and 2.
   *
   * Step 1 here differs from the book in that it returns the most
   * significant bit counting from the right (and starting the count
   * with 0), and then simply OR's the k+1..32 bits of I[xid] and the
   * number 2^k.
   */
  k = MSB(I[xid] ^ I[yid]);
  mask = ~0 << (k + 1);
  b = (I[xid] & mask) | (1 << k);

  mask = ~0 << h(b);
  j = h( (A[xid] & A[yid]) & mask );

#ifdef STATS
  lca->num_compares++;
#endif

  /*
   * Step 3.
   */
  l = h(A[xid]);
  if (l == j)
    xbar = x;
  else {
    mask = ~(~0 << j);       /* The bit-complement of setting bits j..32 */
    k = MSB(A[xid] & mask);

    mask = ~0 << (k + 1);
    Iw = (I[xid] & mask) | (1 << k);
    w = L[Iw];
    xbar = stree_get_parent(tree, w);
  }

#ifdef STATS
  lca->num_compares++;
#endif

  /*
   * Step 4.
   */
  l = h(A[yid]);
  if (l == j)
    ybar = y;
  else {
    mask = ~(~0 << j);
    k = MSB(A[yid] & mask);

    mask = ~0 << (k + 1);
    Iw = (I[yid] & mask) | (1 << k);
    w = L[Iw];
    ybar = stree_get_parent(tree, w);
  }

#ifdef STATS
  lca->num_compares++;
#endif

  /*
   * Step 5.
   */
#ifdef STATS
  lca->num_compares++;
#endif

  if (stree_get_ident(tree, xbar) < stree_get_ident(tree, ybar))
    return xbar;
  else
    return ybar;
}


void lca_free(LCA_STRUCT *lca)
{
  if (lca->I != NULL)
    free(lca->I);
  if (lca->A != NULL)
    free(lca->A);
  if (lca->L != NULL)
    free(lca->L);

  free(lca);
}




/*
 *
 *
 * A Naive LCA Algorithm
 *
 *
 */

/*
 * lca_naive_prep
 *
 * Preprocessing for the naive LCA algorithm.  Nothing really done
 * here, except allocating the LCA_STRUCT structure.
 *
 * Parameters:  tree  -   a suffix tree
 *
 * Returns:  An LCA_STRUCT structure
 */
LCA_STRUCT *lca_naive_prep(SUFFIX_TREE tree)
{
  LCA_STRUCT *lca;

  if (tree == NULL)
    return NULL;

  if ((lca = malloc(sizeof(LCA_STRUCT))) == NULL)
    return NULL;

  memset(lca, 0, sizeof(LCA_STRUCT));
  lca->type = LCA_NAIVE;
  lca->tree = tree;

  return lca;
}


/*
 * lca_naive_lookup
 *
 * Compute the LCA of two suffix tree nodes, using the naive algorithm
 * of walking up the two paths from the nodes to the root until arriving
 * at a common node on both paths.
 *
 * This works because the suffix tree identifiers are given in a depth-first
 * search manner.  So, repeatedly taking the node with the higher numbered
 * identifier and moving to its parent (until the two identifiers are
 * equal) will find the least common ancestor.
 *
 * Parameters:  lca  -  an LCA_STRUCT structure
 *              x    -  a suffix tree node
 *              y    -  another suffix tree node
 *
 * Returns:  the suffix tree node which is the LCA of x and y
 */
STREE_NODE lca_naive_lookup(LCA_STRUCT *lca, STREE_NODE x, STREE_NODE y)
{
  int xid, yid;
  SUFFIX_TREE tree;

  if (lca == NULL || lca->type != LCA_NAIVE || x == NULL || y == NULL)
    return NULL;

  tree = lca->tree;

  xid = stree_get_ident(tree, x);
  yid = stree_get_ident(tree, y);
  while (xid != yid) {
    while (xid > yid) {
      x = stree_get_parent(tree, x);
      xid = stree_get_ident(tree, x);

#ifdef STATS
      lca->num_compares++;
#endif
    }

    while (xid < yid) {
      y = stree_get_parent(tree, y);
      yid = stree_get_ident(tree, y);

#ifdef STATS
      lca->num_compares++;
#endif
    }
  }

#ifdef STATS
  lca->num_compares++;
#endif

  return x;
}


/*
 * lca_naive_free
 *
 * Free the LCA_STRUCT structure.
 *
 * Parameters:  lca  -  an LCA_STRUCT structure.
 *
 * Returns:  nothing
 */
void lca_naive_free(LCA_STRUCT *lca)
{
  free(lca);
}

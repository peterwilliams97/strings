/*
 * stree_lca.c
 *
 * Implementations of several least common ancestor algorithms for
 * suffix trees.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stree_strmat.h"
#include "stree_lca.h"

/*
 * Functions handling the finding of the least and most significant
 * bits of a number (used by the LCA algorithm).
 *
 * The functions are not completely general, as given in the book, but
 * assume that the numbers are 32 bits or less, and are implemented
 * using an 8 bit table and checking successive bytes of each number.
 */
static unsigned int msb_table[256], lsb_table[256];
static bool init_flag = false;

/*
 * init_tables
 *
 * Initialize the tables so that the msb_table and lsb_table contain
 * the most and least significant bit positions for the numbers 1..255.
 */
static void init_tables()
{
    int j = 1;
    int msb = -1;
    for (int i = 1; i < 256; i++) {
        if (i == j) {
            j <<= 1;
            msb++;
        }
        msb_table[i] = msb;

        int lsb, mask;
        for (lsb=0, mask=1; !(mask & i); lsb++,mask<<=1) 
            ;;
        lsb_table[i] = lsb;
    }

    init_flag = true;
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
 * compute_I_and_L
 *
 * Compute the I values and L values for the LCA preprocessing.  
 *  The I values are, for each node, the identifier with the largest least
 *   significant 1 bit in the subtree rooted at the node.  
 *  The L values are,for each node corresponding to an I value, the node 
 *   at the head of each "run" in the tree.
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
    // Shift idents so that they go from 1..num_nodes.
    unsigned int id = (unsigned int)stree_get_ident(tree, node) + 1;

    // Find the node with the maximum I value in the subtree.
    unsigned int Imax = id;
    for (STREE_NODE child = stree_get_children(tree, node); child; child = stree_get_next(tree, child)) {
        unsigned int Ival = compute_I_and_L(lca, tree, child);
        if (h(Ival) > h(Imax))
            Imax = Ival;
        IF_STATS(lca->num_prep++);
    }

    lca->I[id] = Imax;
    lca->L[Imax] = node;    // will be overwritten by the highest node in run 

    return Imax;
}

/*
 * compute_A
 *
 * Computes the A values for the LCA preprocessing.  
 *  The A values are, for each node, the heights of the least significant 
 *   bits of the ancestors of the node (where the bits of each A value are 
 *   set to 1 for each such height of an ancestor).
 *
 * Parameters:  lca    -  an LCA_STRUCT structure
 *              tree   -  a suffix tree
 *              node   -  a suffix tree node
 *              Amask  -  the bits set by the ancestors of node
 */
static void compute_A(LCA_STRUCT *lca, SUFFIX_TREE tree, STREE_NODE node, unsigned int Amask)
{
    // Shift idents so that they go from 1..num_nodes.
    unsigned int id = (unsigned int)stree_get_ident(tree, node) + 1;

    Amask |= 1 << h(lca->I[id]);
    lca->A[id] = Amask;

    for (STREE_NODE child = stree_get_children(tree, node); child; child = stree_get_next(tree, child)) {
        compute_A(lca, tree, child, Amask);
        IF_STATS(lca->num_prep++);
    }
}

/*
 * The Constant Time LCA Algorithm
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
    assert(tree);

    LCA_STRUCT *lca = (LCA_STRUCT *)my_calloc(sizeof(LCA_STRUCT), 1);
    if (!lca) {
        return NULL;
    }

    lca->type = LCA_LINEAR;
    lca->tree = tree;

    int num_nodes = (int)stree_get_num_nodes(tree) + 1;

    if ((lca->I = (unsigned int *)my_calloc(num_nodes, sizeof(unsigned int))) == NULL) {
        lca_free(lca);
        return NULL;
    }
  
    if ((lca->A = (unsigned int *)my_calloc(num_nodes, sizeof(unsigned int))) == NULL) {
        lca_free(lca);
        return NULL;
    }

    if ((lca->L = (STREE_NODE *)my_calloc(num_nodes, sizeof(STREE_NODE))) == NULL) {
        lca_free(lca);
        return NULL;
    }

    // Compute the I and L values, then compute the A values.
    compute_I_and_L(lca, tree, stree_get_root(tree));
    compute_A(lca, tree, stree_get_root(tree), 0);
    
    return lca;
}

// All bits from b upwards including b are on
// e.g.  HIGH_BITS(1) = 111...1110
//       HIGH_BITS(2) = 111...1100
#define HIGH_BITS(b) (~0 << (b))

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
    assert(lca && lca->tree && lca->type == LCA_LINEAR && x && y);
   
    SUFFIX_TREE tree = lca->tree;
    unsigned int *I = lca->I;
    unsigned int *A = lca->A;
    STREE_NODE *L = lca->L;
      
    // Shift idents so that they go from 1..num_nodes.
    unsigned int xid = (unsigned int)stree_get_ident(tree, x) + 1;
    unsigned int yid = (unsigned int)stree_get_ident(tree, y) + 1;

   /*
    * Steps 1 and 2.
    *
    * Step 1 here differs from the book in that it returns the most
    * significant bit counting from the right (and starting the count
    * with 0), and then simply OR's the k+1..32 bits of I[xid] and the
    * number 2^k.
    */
    //printf("xid=%3d, I[xid]=%3d\n", xid, I[xid]); 
    // printf("yid=%3d, I[yid]=%3d\n", yid, I[yid]); 
    unsigned int k = MSB(I[xid] ^ I[yid]);
    unsigned int b = (I[xid] & HIGH_BITS(k+1)) | (1 << k);
    unsigned int j = h( (A[xid] & A[yid]) & HIGH_BITS(h(b)) );
//    printf("k=%d, b=%d, j =%d\n", k, b, j);

    IF_STATS(lca->num_compares++);

    // Step 3.
    unsigned int l = h(A[xid]);
    STREE_NODE xbar, ybar; 
    if (l == j) {
        xbar = x;
    } else {
        k = MSB(A[xid] & ~HIGH_BITS(j));
        xbar = stree_get_parent(tree, L[(I[xid] & HIGH_BITS(k+1)) | (1 << k)]);
    }
    IF_STATS(lca->num_compares++);
   
    //  Step 4.
    l = h(A[yid]);
    if (l == j) {
        ybar = y;
    } else {
        k = MSB(A[yid] & ~HIGH_BITS(j));
        ybar = stree_get_parent(tree, L[(I[yid] & HIGH_BITS(k+1)) | (1 << k)]);
    }
    IF_STATS(lca->num_compares++);

    // Step 5.
    IF_STATS(lca->num_compares++);

    return (stree_get_ident(tree, xbar) < stree_get_ident(tree, ybar)) ? xbar : ybar;
}

void lca_free(LCA_STRUCT *lca)
{
    free(lca->I);
    free(lca->A);
    free(lca->L);
    free(lca);
}

/*
 * A Naive LCA Algorithm
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

    if ((lca = (LCA_STRUCT *)my_calloc(sizeof(LCA_STRUCT), 1)) == NULL)
        return NULL;

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
    assert (lca && lca->type == LCA_NAIVE && x && y);

    SUFFIX_TREE tree = lca->tree;

    int xid = stree_get_ident(tree, x);
    int yid = stree_get_ident(tree, y);
    while (xid != yid) {
        
        while (xid > yid) {
            x = stree_get_parent(tree, x);
            xid = stree_get_ident(tree, x);
            IF_STATS(lca->num_compares++);
        }

        while (xid < yid) {
            y = stree_get_parent(tree, y);
            yid = stree_get_ident(tree, y);
            IF_STATS(lca->num_compares++);
        }
    }

    IF_STATS(lca->num_compares++);
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

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "strmat.h"
#include "strmat_print.h"
#include "strmat_util.h"
#include "stree_ukkonen.h"
#include "stree_lca.h"
#include "peter_longest_common_extension.h"

using namespace std;

/*
 * compute_nodemap
 *
 * Compute the mapping from identifiers to suffix tree nodes and store
 * that mapping in "nodemap"
 *
 * Parameters:  tree     -  a suffix tree
 *              node     -  a suffix tree node
 *              nodemap  -  the nodemap being computed
 */
static void compute_nodemap(SUFFIX_TREE tree, STREE_NODE node, STREE_NODE *nodemap)
{
    nodemap[stree_get_ident(tree, node)] = node;

    for (STREE_NODE child = stree_get_children(tree, node); child; child = stree_get_next(tree, child)) {
        compute_nodemap(tree, child, nodemap);
    }
}

LCE *prepare_longest_common_extension(STRING *s1, STRING *s2, bool print_stats)
{
     assert(s1 && s2);

    // Build the suffix tree.
    printf("Building the tree...\n\n");

    bool ok = true;
    int num_strings = 2;
    STRING *strings[2];
    strings[0] = s1;
    strings[1] = s2;
    SUFFIX_TREE tree = stree_gen_ukkonen_build(strings, num_strings, &ok, print_stats);
    if (!tree) {
        return false;
    }
    
    printf("\nSuffix Tree:\n");
    small_print_tree(tree, stree_get_root(tree), 0, 2);
   
    // Build the LCA tables
    LCE *lce = (LCE *)my_calloc(sizeof(LCE),1);
    lce->_lca = lca_prep(tree);

     // Build the map of suffix tree nodes.
    lce->_nodemap = (STREE_NODE *)my_calloc((int)stree_get_num_nodes(tree), sizeof(STREE_NODE));
    compute_nodemap(tree, stree_get_root(tree), lce->_nodemap);

    return lce;
}

 // Free everything allocated.
void longest_common_extension_free(LCE *lce)
{
    stree_delete_tree(lce->_lca->tree);
    lca_free(lce->_lca);
    free(lce->_nodemap);
    free(lce);
}

/*
 * Return longest common extension of
 *      offset ofs1 into string lce->s1
 *      offset ofs2 into string lce->s2
 */
STREE_NODE lookup_lce(LCE *lce, int ofs1, int ofs2)
{
    SUFFIX_TREE tree = lce->_lca->tree;
    STREE_NODE x = lce->_nodemap[ofs1+1];
    STREE_NODE y = lce->_nodemap[ofs2+1];
    print_label(tree, x, "x=");
    print_label(tree, y, "y=");
    return lca_lookup(lce->_lca, x,  y);
}


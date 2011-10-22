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
  //  int flag, pos, matchlen;
//#ifdef STATS
//    _int64 num_compares, edges_traversed, child_cost;
//#endif
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
   
   /* int max_length = 0;
    for (int i = 0; i < num_strings; i++) {
        if (strings[i]->length > max_length)
            max_length = strings[i]->length;
    }*/

    LCE *lce = (LCE *)my_calloc(sizeof(LCE),1);
    lce->_lca = lca_prep(tree);

     // Build the map of suffix tree nodes.
    int num_nodes = (int)stree_get_num_nodes(tree);
    lce->_nodemap = (STREE_NODE *)my_calloc((int)num_nodes, sizeof(STREE_NODE));
    compute_nodemap(tree, stree_get_root(tree), lce->_nodemap);

    return lce;
}

void longest_common_extension_free(LCE *lce)
{
    // Free everything allocated.
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
    STREE_NODE x = lce->_nodemap[ofs1];
    STREE_NODE y = lce->_nodemap[ofs2];
    print_node(x, "x=");
    print_node(y, "y=");
    return lca_lookup(lce->_lca, x,  y);
}


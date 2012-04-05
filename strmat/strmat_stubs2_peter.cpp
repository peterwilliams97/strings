#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <iostream>
#include <string>
#include "strmat.h"
#include "strmat_match.h"
#include "strmat_print.h"
#include "strmat_util.h"
#include "stree_strmat.h"
#include "stree_ukkonen.h"
//#include "stree_weiner.h"
#include "stree_lca.h"
//#include "stree_decomposition.h"
#include "strmat_stubs2.h"

using namespace std;
/*
 * strmat_ukkonen_build
 *
 * Performs the Ukkonen suffix tree construction algorithm for
 * one or more strings.
 *
 * Parameters:   strings          -  the input strings
 *               num_strings      -  the number of input strings
 *               print_stats      -  flag telling whether to print the stats
 *               print_tree       -  flag telling whether to print the tree
 *
 * Returns: true on success
 */
bool strmat_ukkonen_build(const STRING **strings, int num_strings, bool print_stats, bool print_tree)
{
    int i;
    _int64  max_length, total_length;
    SUFFIX_TREE tree;
 
    assert(strings);
 
    // Find the total and maximum length of the input strings.
    max_length = -1;
    total_length = 0;
    for (i=0; i < num_strings; i++) {
        total_length += strings[i]->length;
        if (max_length == -1 || strings[i]->length > max_length)
            max_length = strings[i]->length;
    }

    /*
    * Build the tree, then print the output.
    */
    bool ok;
    tree = stree_gen_ukkonen_build(strings, num_strings, &ok, print_tree);
    if (tree == NULL)
        return false;

    if (print_stats) {
        printf("\nStatistics:\n");
#ifdef STATS
        printf("   Sum of Sequence Sizes:       %9I64d (%7.1f MB)\n", total_length, mb(total_length));
        printf("   Number of Tree Nodes:        %9I64d (%7.1f M)\n", stree_get_num_nodes(tree), mb(stree_get_num_nodes(tree)));
        printf("   Size of Optimized Tree:      %9I64d (%7.1f MB)\n", tree->tree_size, mb(tree->tree_size));
        printf("   Bytes per Character:         %9.2f\n",  (float) tree->tree_size / (float) total_length);
        printf("\n");
        printf("   Number of Comparisons:       %9I64d\n", tree->num_compares);
        printf("   Cost of Constructing Edges:  %9I64d\n", tree->creation_cost);
        printf("   Number of Edges Traversed:   %9I64d\n", tree->edges_traversed);
        printf("   Cost of Traversing Edges:    %9I64d\n", tree->child_cost);
        printf("   Number of Links Traversed:   %9I64d\n", tree->links_traversed);
#else
        printf("   No statistics available.\n");
#endif
    }

    if (print_tree) {
        printf("\nSuffix Tree:\n");
        if (max_length < 40 || true)
            small_print_tree(tree, stree_get_root(tree), 0, (num_strings > 1));
        else
            large_print_tree(tree, stree_get_root(tree), (num_strings > 1));
        mputc('\n');
    }

    stree_delete_tree(tree);
    return ok;
}

static MATCHES matchlist;
static int matchcount, matcherror, patlen;

/*
 * preorder_fn used to implement strmat_stree_match() with stree_traverse_subtree()
 */
static int add_match(SUFFIX_TREE tree, STREE_NODE node)
{
   // cout << " add_match(): node.id=" << node->id << endl;
    
    int pos, id;
    CHAR_TYPE *seq;
     
    for (int i = 1; stree_get_leaf(tree, node, i, &seq, &pos, &id); i++) {
        MATCHES newmatch = alloc_match();
        if (newmatch == NULL) {
            free_matches(matchlist);
            matchlist = NULL;
            matcherror = 1;
            return 0;
        }

   //     cout << "   stree_get_leaf: i=" << i << ",pos=" << pos << ",id=" << id << endl;
        // Shift positions by 1 here (from 0..N-1 to 1..N).
        newmatch->type = TEXT_SET_EXACT;
        newmatch->lend = pos + 1;
        newmatch->rend = pos + patlen;
        newmatch->textid = id;

        newmatch->next = matchlist;
        matchlist = newmatch;
        matchcount++;
    }

    return 1;
}

/*
 * strmat_stree_match
 *
 * Perform an exact matching of a pattern to a suffix tree of one or
 * more strings.
 *
 * Parameters:   pattern          -  the input pattern
 *               strings          -  the input strings
 *               num_strings      -  the number of input strings
 *               print_stats      -  flag telling whether to print the stats
 *
 * Returns:  true on success
 */
bool strmat_stree_match(const STRING *pattern, const STRING **strings, int num_strings, bool print_stats, bool do_print_matches)
{
    int flag, pos, matchlen;
#ifdef STATS
    _int64 num_compares, edges_traversed, child_cost;
#endif
    MATCHES back, current, next;
    STREE_NODE node;
    SUFFIX_TREE tree;
    bool ok = true;

    assert(pattern && strings);

    // Build the suffix tree.
    printf("Building the tree...\n\n");
    tree = stree_gen_ukkonen_build(strings, num_strings, &ok, print_stats);
    if (tree == NULL)
        return false;

    stree_reset_stats(tree);

    // Match the pattern string to a path in the suffix tree.
    matchlen = stree_match(tree, pattern->sequence, pattern->length, &node, &pos);
    if (matchlen < 0) {
        stree_delete_tree(tree);
        return false;
    }

#ifdef STATS
    num_compares = tree->num_compares;
    edges_traversed = tree->edges_traversed;
    child_cost = tree->child_cost;
    stree_reset_stats(tree);
#endif

    // Traverse the subtree, finding the matches.
    matchlist = NULL;
    matchcount = matcherror = 0;
    patlen = pattern->length;

    if (matchlen == pattern->length) {
        stree_traverse_subtree(tree, node, add_match, NULL);
        if (matcherror) {
            stree_delete_tree(tree);
            return false;
        }
      
        // Bubble sort the matches.
        flag = 1;
        while (flag) {
            flag = 0;
            back = NULL;
            current = matchlist;
            while (current->next != NULL) {
                if (current->next->textid < current->textid || (current->next->textid == current->textid  &&  current->next->lend < current->lend)) {
                    // Move current->next before current in the list.
                    next = current->next;
                    current->next = next->next;
                    next->next = current;
                    if (back == NULL)
                        back = matchlist = next;
                    else
                        back = back->next = next;
                    flag = 1;
                } else {
                    back = current;
                    current = current->next;
                }
            }
        }
    }


    // Print the matches and the statistics.
    if (do_print_matches) {
   
        printf("\nSuffix Tree:\n");
        small_print_tree(tree, stree_get_root(tree), 0, (num_strings > 1));

        print_matches(NULL, strings, num_strings, matchlist, matchcount);
    }

    if (print_stats) {
        printf("Statistics:\n");
#ifdef STATS
        printf("   Matching:\n");
        printf("      Pattern Length:          %d\n", pattern->length);
        printf("      Number of Comparisons:   %d\n", num_compares);
        printf("      Number Edges Traversed:  %d\n", edges_traversed);
        printf("      Cost of Edge Traversal:  %d\n", child_cost);
        printf("\n");
        printf("   Subtree Traversal:\n");
        printf("      Number of Matches:       %d\n", matchcount);
        printf("      Number Edges Traversed:  %d\n", tree->edges_traversed);
        printf("      Cost of Edge Traversal:  %d\n", tree->child_cost);
#else
        printf("   No statistics available.\n");
#endif
        mputc('\n');
    }

    // Free everything allocated.
    free_matches(matchlist);
    stree_delete_tree(tree);

    return true;
}

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

/*
 * strmat_stree_lca
 *
 * Performs the constant time LCA algorithm on a suffix tree for a
 * string.
 *
 * Parameters:   strings          -  the input strings
 *               num_strings      -  the number of input strings
 *               print_stats      -  flag telling whether to print the stats
 *
 * Returns:  non-zero on success, zero on error
 */
bool int_strmat_stree_lca(const STRING **strings, int num_strings, bool print_stats, LCA_TYPE type, char *lines[])
{
    int num1, num2, len, num_lcas, max_length;
    _int64 num_nodes;
    CHAR_TYPE *s;
    char **line_ptr;
    CHAR_TYPE buffer[64];
    STREE_NODE x, y, z, *nodemap;
    SUFFIX_TREE tree;
    LCA_STRUCT *lcastruct;
    
    assert(strings);
    
    // Build the tree.
   
    printf("Building the suffix tree...\n");
    bool ok = true;
    tree = stree_gen_ukkonen_build(strings, num_strings, &ok, print_stats);
    if (tree == NULL)
        return false;

    num_nodes = stree_get_num_nodes(tree);
    max_length = -1;
    int i;
    for (i = 0; i < num_strings; i++) {
        if (max_length == -1 || strings[i]->length > max_length)
            max_length = strings[i]->length;
    }

    // Preprocess the suffix tree.
    printf("Preprocessing...\n");
    lcastruct = NULL;
    switch (type) {
        case LCA_NAIVE:   lcastruct = lca_naive_prep(tree);  break;
        case LCA_LINEAR:  lcastruct = lca_prep(tree);  break;
        case LCA_NLOGN:   return 0;
    }
    if (lcastruct == NULL) {
        stree_delete_tree(tree);
        return 0;
    }

    // Build the map of suffix tree nodes.
    nodemap = (STREE_NODE *)my_calloc((int)num_nodes, sizeof(STREE_NODE));
    if (nodemap == NULL) {
        lca_free(lcastruct);
        stree_delete_tree(tree);
        return 0;
    }

    compute_nodemap(tree, stree_get_root(tree), nodemap);

    // Query the user to enter nodes, and compute the LCA of those nodes.
    // !@#$ Now passed in as an argument
    printf("\n");
    printf("Commands (0-%d 0-%d - Find LCA of two nodes (identify by number),\n",
         num_nodes-1, num_nodes-1);
    printf("          ! - print suffix tree, Ctl-D - quit)\n");

    num_lcas = 0;
    for (line_ptr = lines; line_ptr; line_ptr++) {
        char *line = *line_ptr;
        if (line[0] == '\0') {
            continue;
        } else if (line[0] == '!') {
            mputc('\n');
            printf("Suffix Tree:\n");
            if (max_length < 40)
                small_print_tree(tree, stree_get_root(tree), 0, (num_strings > 1));
            else
                large_print_tree(tree, stree_get_root(tree), (num_strings > 1));
            mputc('\n');
        }  else if (sscanf(line, "%d %d", &num1, &num2) == 2 
                && num1 >= 0 && num1 < num_nodes 
                && num2 >= 0 && num2 < num_nodes) {
            x = nodemap[num1];
            y = nodemap[num2];
            z = NULL;
            switch (type) {
                case LCA_NAIVE:   z = lca_naive_lookup(lcastruct, x, y);  break;
                case LCA_LINEAR:  z = lca_lookup(lcastruct, x, y);  break;
                case LCA_NLOGN:  return 0;
            }
            num_lcas++;
            
            if (x == stree_get_root(tree)) {
                printf("   Node %d:  (root)\n", stree_get_ident(tree, x));
            } else {
                len = stree_get_labellen(tree, x);
                stree_get_label(tree, x, buffer, 50, 0);
                for (s=buffer,i=0; *s && i < 50; s++,i++) {
                    if (!isprint((int)(*s)))
                        *s = '#';
                }
                if (len > 50) {
                    buffer[50] = buffer[51] = buffer[52] = '.';
                    buffer[53] = '\0';
                } else if (len == 50) {
                    buffer[50] = '\0';
                }
                printf("   Node %d:  %s\n", stree_get_ident(tree, x), buffer);
            }

            if (y == stree_get_root(tree)) {
                printf("   Node %d:  (root)\n", stree_get_ident(tree, y));
            } else {
                len = stree_get_labellen(tree, y);
                stree_get_label(tree, y, buffer, 50, 0);
                for (s=buffer,i=0; *s && i < 50; s++,i++)
                    if (!isprint((int)(*s)))
                        *s = '#';
                if (len > 50) {
                    buffer[50] = buffer[51] = buffer[52] = '.';
                    buffer[53] = '\0';
                }  else if (len == 50) {
                    buffer[50] = '\0';
                }
                printf("   Node %d:  %s\n", stree_get_ident(tree, y), buffer);
            }

            if (z == stree_get_root(tree)) {
                printf("   LCA Node %d:  (root)\n", stree_get_ident(tree, z));
            } else {
                len = stree_get_labellen(tree, z);
                stree_get_label(tree, z, buffer, 50, 0);
                for (s=buffer,i=0; *s && i < 50; s++,i++)
                    if (!isprint((int)(*s)))
                        *s = '#';
                if (len > 50) {
                    buffer[50] = buffer[51] = buffer[52] = '.';
                    buffer[53] = '\0';
                }  else if (len == 50) {
                    buffer[50] = '\0';
                }
                printf("   LCA Node %d:  %s\n", stree_get_ident(tree, z), buffer);
            }
            
            putchar('\n');
    
        } 
    }

    if (print_stats) {
        printf("\nStatistics:\n");
#ifdef STATS
        printf("   Preprocessing Steps:    %d\n", lcastruct->num_prep);
        printf("\n");
        printf("   Number LCA's Computed:  %d\n", num_lcas);
        printf("   LCA Compute Steps:      %d\n", lcastruct->num_compares);
#else
        printf("   No statistics available.\n");
#endif
       putchar('\n');
    }

    free(nodemap);
    switch (type) {
        case LCA_NAIVE:   lca_naive_free(lcastruct);  break;
        case LCA_LINEAR:  lca_free(lcastruct);  break;
        case LCA_NLOGN:  return 0;
    }
    stree_delete_tree(tree);

    return 1;
}

bool strmat_stree_lca(const STRING **strings, int num_strings, bool print_stats, char *lines[])
{  
    return int_strmat_stree_lca(strings, num_strings,  print_stats, LCA_LINEAR, lines);  
}

bool strmat_stree_naive_lca(const STRING **strings, int num_strings, bool print_stats, char *lines[])
{  
    return int_strmat_stree_lca(strings, num_strings, print_stats, LCA_NAIVE, lines);  
}


/*
 * strmat_stree_walkaround
 *
 * Interactively walk around the nodes of a suffix tree.
 *
 * Parameters:   strings          -  the input strings
 *               num_strings      -  the number of input strings
 *
 * Returns:  non-zero on success, zero on error
 */
static void print_stree_node(SUFFIX_TREE tree, STREE_NODE node, int gen_stree_flag, int mend_num_lines);

//int strmat_stree_walkaround(STRING **strings, int num_strings)
//{
//    char mapch, *choice;
//    STREE_NODE node, child;
//    SUFFIX_TREE tree;
//
//    if (strings == NULL)
//        return 0;
//
//  /*
//   * Build the tree.
//   */
//  printf("Building the suffix tree...\n");
//  tree = stree_gen_ukkonen_build(strings, num_strings);
//  if (tree == NULL)
//    return 0;
//
// 
//  /*
//   * Main interactive loop.
//   */
//  node = stree_get_root(tree);
//  while (1) {
//    /*
//     * Print the details of the current node.
//     */
//    printf("\n\n");
//    print_stree_node(tree, node, (num_strings > 1), 5);
//
//    /*
//     * Ask the user where to move in the tree.
//     */
//    printf("\n");
//    printf("Commands (d%% - move down to a child, u - move up to parent,\n");
//    printf("          l - move across suffix link, Ctl-D - quit)\n");
//    printf("Enter Move: ");
//
//    if ((choice = my_getline(stdin, NULL)) == NULL)
//      break;
//
//    if (choice[0] == '\0')
//      continue;
//
//    /*
//     * Execute the move command.
//     */
//    switch (toupper(choice[0])) {
//    case 'D':
//      if (choice[1] == '\0') {
//        printf("\nYou must specify the first character on an edge to a child.\n");
//        continue;
//      }
//
//      mapch = choice[1];
//
//      if ((child = stree_find_child(tree, node, mapch)) == NULL) {
//        printf("\nNo child's edge begins with '%c'.\n", choice[1]);
//        continue;
//      }
//
//      node = child;
//      break;
//
//    case 'U':
//      if (node == stree_get_root(tree))
//        printf("\nYou cannot move up from the root.\n");
//      else
//        node = stree_get_parent(tree, node);
//      break;
//
//    case 'L':
//      if (node == stree_get_root(tree))
//        printf("\nThe root has no suffix link.\n");
//      else
//        node = stree_get_suffix_link(tree, node);
//      break;
//
//    default:
//      printf("\nThat is not a choice.\n");
//    }
//  }
//
//  stree_delete_tree(tree);
//
//  return 1;
//}


/*
 * print_stree_node
 *
 * Print the details about a node of a suffix tree.
 *
 * Parameters:  tree            -  a suffix tree
 *              node            -  a tree node
 *              gen_stree_flag  -  is this a generalized suffix tree?
 *              mend_num_lines  -  how many lines will appear after this
 *
 * Returns: nothing
 */
static void print_stree_node(SUFFIX_TREE tree, STREE_NODE node, int gen_stree_flag, int mend_num_lines)
{
  int i, j, index, ident, idwidth, labellen, edgelen;
  int leafnum, pos;
  CHAR_TYPE *edgestr;
  CHAR_TYPE label[36];
  CHAR_TYPE *str;
  STREE_NODE child;

 
  /*
   * Get the node and edge information.
   */
  labellen = stree_get_labellen(tree, node);
  stree_get_label(tree, node, label, 30, 1);
  for (i=0; i < 30 && label[i]; i++)
    if (!isprint((int)label[i]))
      label[i] = '#';
  label[i] = '\0';

  edgelen = stree_get_edgelen(tree, node);

  ident = stree_get_ident(tree, node);
  idwidth = my_itoalen(ident);

  /*
   * Print the node info: ident, label, edge label, leaves & suffix link.
   */
  if (node == stree_get_root(tree))
    printf("Current node is Node %d, the Root\n", ident);
  else {
    printf("Current node is Node %d, labeled `%s%s'\n", ident,
            (labellen > 30 ? "..." : ""), label);
    printf("     Leaves:  ");
    leafnum = 1;
    for (;stree_get_leaf(tree, node, leafnum, &str, &pos, &index); leafnum++) {
      if (leafnum > 1)
        mputs(", ");

      if (gen_stree_flag)
        printf("%d:%d", index, pos + 1);
      else
        printf("%d", pos + 1);
    }
    if (leafnum == 1)
      mputs("(none)");
    mputc('\n');
    printf("       Edge:  %s%s\n", (edgelen > 30 ? "..." : ""),
            (edgelen > 30 ? label : (label + (labellen - edgelen))));
    printf("     Parent:  Node %d\n",
            stree_get_ident(tree, stree_get_parent(tree, node)));
    printf("  Suf. Link:  Node %d\n",
            stree_get_ident(tree, stree_get_suffix_link(tree, node)));
  }

  /*
   * Print the outgoing edges.
   */
  if (stree_get_num_children(tree, node) == 0)
    printf("   Children:\n       (none)\n");
  else {
    printf("   Children:\n");
    child = stree_get_children(tree, node);
    while (child != NULL) {
      edgestr = stree_get_edgestr(tree, child);
      edgelen = stree_get_edgelen(tree, child);

      for (j=0; j < 30 && j < edgelen; j++)
        label[j] = (isprint((int)edgestr[j]) ? edgestr[j] : '#');
      label[j] = '\0';

      if (edgelen > 30) {
        label[30] = label[31] = label[32] = '.';
        label[33] = '\0';
      }

      printf("       %s  ->  Node %d", label,
              stree_get_ident(tree, child));

      leafnum = 1;
      while (stree_get_leaf(tree, child, leafnum, &str, &pos, &index)) {
        if (leafnum == 1)
          printf("    (Leaf #");
        else
          printf(", ");

        if (gen_stree_flag)
          printf("%d:%d", index, pos + 1);
        else
          printf("%d", pos + 1);

        leafnum++;
      }
      if (leafnum > 1)
        mputc(')');

      mputc('\n');

      child = stree_get_next(tree, child);
    }
  }

}

/*
 * strmat_stree_lempel_ziv
 *
 * Compute Lempel-Ziv decomposition of a string.
 *
 * Parameters:   string           -  the input string
 *               build_policy     -  suffix tree build policy
 *               build_threshold  -  threshold used by LIST_THEN_ARRAY
 *               print_stats      -  flag telling whether to print the stats
 *               mode             -  flag telling whether
 *                                     A) most efficient decomposition
 *                                     B) original Lempel-Ziv
 *                                     C) non-overlapping Lempel-Ziv
 *
 * Returns:  non-zero on success, zero on error
 */
//int strmat_stree_lempel_ziv(STRING *string, int build_policy,
//                            int build_threshold, int print_stats,
//                            char mode)
//{
//  SUFFIX_TREE tree;
//  DECOMPOSITION_STRUCT *decomp_struct;
//
//  if (string == NULL || string->sequence == NULL || string->length == 0)
//    return 0;
//
//  /*
//   * Build the tree (can't use stree_ukkonen_build because of copyflag).
//   */
//  printf("\nBuilding the suffix tree...\n");
//  if ((tree = stree_new_tree(string->alpha_size, 0, build_policy,
//                             build_threshold)) == NULL)
//    return 0;
//
//  if (stree_ukkonen_add_string(tree, string->sequence, string->raw_seq,
//                               string->length, 1) <= 0) {
//    stree_delete_tree(tree);
//    return 0;
//  }
//
//  /*
//   * Preprocess the suffix tree and initialize repeat_struct
//   */
//  printf("Preprocessing...\n");
//  decomp_struct = decomposition_prep(tree, string->sequence,
//                                     string->raw_seq, string->length);
//  if(decomp_struct == NULL) {
//    stree_delete_tree(tree);
//    return 0;
//  }
//
//  /*
//   * Compute the decomposition
//   */
//  printf("Computing the decomposition...\n");
//  if(mode == 'A')
//    lempel_ziv(decomp_struct);
//  else if(mode == 'B')
//    lempel_ziv_nonoverlapping(decomp_struct);
//  else {
//    fprintf(stderr,"Unknown mode `%d'.\n",mode);
//    return 0;
//  }
//
//  /*
//   * Print Lempel-Ziv decomposition
//   */
//  printf("\nThe %sLempel-Ziv decomposition is:\n\n",
//          mode=='A' ? "" : mode=='B' ? "original " : "non-overlapping ");
//  decomposition_print(decomp_struct);
//  printf("\n");
//  mend(14);
//
//  /* Write summary of results */
//  mstart(stdin,stdout,OK,OK,0,NULL);
//  printf("\nSummary:\n");
//  printf("   Number of Blocks:              %d\n",decomp_struct->num_blocks);
//  printf("   Maximal Block Length:          %d\n",
//          decomp_struct->max_block_length);
//  printf("   Average Block Length:          %.1f\n",
//          (float)decomp_struct->length / decomp_struct->num_blocks);
//
//  /*
//   * Write statistics and free memory
//   */
//  if (print_stats) {
//    printf("\nStatistics:\n");
//#ifdef STATS
//    printf("   String Length:                 %d\n",decomp_struct->length);
//    printf("   Suffix Tree\n");
//    printf("      Number of Tree Nodes:       %d\n",
//            stree_get_num_nodes(tree));
//    printf("      Number of Compares:         %d\n",tree->num_compares);
//    printf("   Decomposition\n");
//    printf("      Number of Compares:         %d\n",
//            decomp_struct->num_compares);
//    printf("      Number of Edge Traversals:  %d\n",
//            decomp_struct->num_edge_traversals);
//#else
//    printf("   No statistics available.\n");
//#endif
//    printf("\n");
//  }
//
//  decomposition_free(decomp_struct);
//  stree_delete_tree(tree);
//
//  return 1;
//}


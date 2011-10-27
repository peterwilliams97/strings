/*
 * stree_strmat.c
 *
 * Implementation of the suffix tree data structure.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <iostream>
#include "stree_strmat.h"
#include "peter_global.h"

using namespace std;

#ifdef STATS
    #define OPT_NODE_SIZE 24
    #define OPT_LEAF_SIZE 12
    #define OPT_INTLEAF_SIZE 12
#endif

static bool node_has_children(STREE_NODE node)
{
 #ifdef PETER_GLOBAL
    return pglob_get_number_children(node->_index) > 0;
#else
    return node->children != NULL;
#endif
}

/*
 * The Suffix Tree Interface Procedures
 */

/*
 * stree_new_tree
 *
 * Allocates a new suffix tree data structure.
 *
 * Parameters:  copyflag    -  make a copy of each sequence?
 *
 * Returns:  A SUFFIX_TREE structure
 */
SUFFIX_TREE stree_new_tree(int copyflag)
{
#ifdef PETER_GLOBAL
    pglob_init();
#endif            

    // Allocate the space.
    SUFFIX_TREE tree = (SUFFIX_TREE)my_calloc(1, sizeof(STREE_STRUCT));
    if (!tree) {
        return NULL;
    }
     
    tree->copyflag = copyflag;
   
    if ((tree->root = int_stree_new_node(tree, NULL, 0)) == NULL) {
        free(tree);
        return NULL;
    }
    tree->num_nodes = 1;
  
    return tree;
}

/*
 * stree_delete_tree
 *
 * Frees the SUFFIX_TREE data structure and all of its allocated space.
 *
 * Parameters:  tree  -  a suffix tree
 *
 * Returns:  nothing.
 */
void stree_delete_tree(SUFFIX_TREE tree)
{
    if (tree == NULL) {
        return;
    }
    int_stree_delete_subtree(tree, stree_get_root(tree));
  
    if (tree->strings != NULL) {
        if (tree->copyflag) {
            for (int i = 0; i < tree->strsize; i++) {
                free(tree->strings[i]);
            }
        }
        free(tree->strings);
    }
      
    free(tree->ids);
    free(tree->lengths);
    free(tree);

#if PETER_GLOBAL
    pglob_clear();
#endif
}

/*
 * stree_traverse & stree_traverse_subtree
 *
 * Use a non-recursive traversal of the tree (or a subtree), calling the
 * two function parameters before and after recursing at each node, resp.
 * When memory is at a premium, this traversal may be useful.
 *
 * Note that either of the function parameters can be NULL, if you just
 * need to do pre-order or post-order processing.
 *
 * The traversal uses the `isaleaf' field of the tree nodes to hold its
 * state information.  Since `isaleaf' will always be 0 at the internal
 * nodes where the state information is needed, this will not affect the
 * values at the nodes (and all changes to `isaleaf' will be undone.
 *
 * Parameters:  tree          -  a suffix tree
 *              node          -  root node of the traversal (stree_traverse_subtree only)
 *              preorder_fn   -  function to call before visiting the children
 *              postorder_fn  -  function to call after visiting all children
 *
 * Returns:  nothing.
 */
void stree_traverse(SUFFIX_TREE tree, int (*preorder_fn)(SUFFIX_TREE,STREE_NODE), int (*postorder_fn)(SUFFIX_TREE,STREE_NODE))
{
    stree_traverse_subtree(tree, stree_get_root(tree), preorder_fn, postorder_fn);
}

void stree_traverse_subtree(SUFFIX_TREE tree, STREE_NODE node, int (*preorder_fn)(SUFFIX_TREE,STREE_NODE), int (*postorder_fn)(SUFFIX_TREE,STREE_NODE))
{
    enum { START, FIRST, MIDDLE, DONE, DONELEAF } state;
    int num, childnum;
    STREE_NODE root, child = NULL;

    // Use a non-recursive traversal where the 'isaleaf' field of each node
    // is used as the value remembering the child currently being traversed.
    root = node;
    state = START;
    while (true) {
        // The first time we get to a node.
        if (state == START) {
            if (preorder_fn) {
                (*preorder_fn)(tree, node);
            }
            num = stree_get_num_children(tree, node);
            state = (num > 0) ? FIRST : DONELEAF;
        }
        // Start or continue recursing on the children of the node.
        if (state == FIRST || state == MIDDLE) {
            // Look for the next child to traverse down to.
            childnum = (state == FIRST) ? 0 : node->isaleaf;
#ifdef PETER_GLOBAL
            // !@#$ Neeed to get this to match non-PETER_GLOBAL
            // !@#$ Simlify this verbosity
            map<CHAR_TYPE, stree_node *> children = pglob_get_children_map(node->_index);
            child = (childnum < (int)children.size()) ? children[pglob_get_children_keys(node->_index)[childnum]] : NULL;
            int i = childnum;
#else
            STREE_NODE *children = (STREE_NODE *)node->children;
            int i;
           // cout << " node.id=" << node->id << endl;
            for (i = childnum; i < ALPHABET_SIZE; i++) {
             //   cout << "  [" << i << ":" << (children[i] != NULL) << "]" << endl;
                if (children[i] != NULL)
                    break;
                IF_STATS(tree->child_cost++);
            }
            child = (i < ALPHABET_SIZE) ? children[i] : NULL;
#endif
            if (child == NULL) {
                state = DONE;
            } else {
                node->isaleaf = i + 1;
                node = child;
                IF_STATS(tree->edges_traversed++);
                state = START;
            }
        }

        // Last time we get to a node, do the post-processing and move back up,
        // unless we're at the root of the traversal, in which case we stop.
        if (state == DONE || state == DONELEAF) {
            if (state == DONE)
                node->isaleaf = 0;
            if (postorder_fn) {
                (*postorder_fn)(tree, node);
            }
            if (node == root)
                break;
            node = stree_get_parent(tree, node);
            state = MIDDLE;
        }
    }
}

/*
 * stree_match & stree_walk
 *
 * Traverse the path down the tree whose path label matches T, and return
 * the number of characters of T matches, and the node and position along
 * the node's edge where the matching to T ends.
 *
 * Parameters:  tree      -  a suffix tree
 *              node      -  what node to start the walk down the tree
 *              pos       -  position along node's edge to start the walk
 *                              (`node' and `pos' are stree_walk only)
 *              T         -  the sequence to match
 *              N         -  the sequence length
 *              node_out  -  address of where to store the node where
 *                           the traversal ends
 *              pos_out   -  address of where to store the character position
 *                           along the ending node's edge of the endpoint of
 *                           the traversal
 *
 * Returns:  The number of characters of T matched.
 */
int stree_walk(SUFFIX_TREE tree, STREE_NODE node, int pos, CHAR_TYPE *T, int N, STREE_NODE *node_out, int *pos_out)
{
    int len, endpos, edgelen;
    CHAR_TYPE *edgestr;
    STREE_NODE endnode;

    len = int_stree_walk_to_leaf(tree, node, pos, T, N, &endnode, &endpos);

    if (!int_stree_isaleaf(tree, endnode) || len == N) {
        *node_out = endnode;
        *pos_out = endpos;
        return len;
    }

    edgestr = stree_get_edgestr(tree, endnode);
    edgelen = stree_get_edgelen(tree, endnode);

    while (len < N && endpos < edgelen && T[len] == edgestr[endpos]) {
        IF_STATS(tree->num_compares++);
        len++;
        endpos++;
    }

    IF_STATS(tree->num_compares++);
    *node_out = endnode;
    *pos_out = endpos;
    return len;
}

int stree_match(SUFFIX_TREE tree, CHAR_TYPE *T, int N, STREE_NODE *node_out, int *pos_out)
{
    return stree_walk(tree, stree_get_root(tree), 0, T, N, node_out, pos_out);
}

/*
 * stree_find_child
 *
 * Find the child of a node whose edge label begins with the character given
 * as a parameter.
 *
 * Parameters:  tree  -  a suffix tree
 *              node  -  a tree node
 *              ch    -  a character
 *
 * Returns:  a tree node or NULL.
 */
STREE_NODE stree_find_child(SUFFIX_TREE tree, STREE_NODE node, CHAR_TYPE ch)
{
    if (int_stree_isaleaf(tree, node) || !node_has_children(node))
        return NULL;

    IF_STATS(tree->child_cost++);

#ifdef PETER_GLOBAL
    STREE_NODE child_node = pglob_get_child_node(node->_index, ch);
#if 0  // !@#$ Peter debugging
    if (child_node) {
        char buffer[CHAR_BUFFER_LEN];
        printf("  child node: %d %s\n", child_node->edgelen, 
            get_char_array(child_node->edgestr, child_node->edgelen, buffer));
    }
#endif
    return child_node;
#else
    STREE_NODE *children = (STREE_NODE *)node->children;
    {   // !@#$ Peter's debugging
        STREE_NODE child = children[ch];
        if (child && !child->edgestr) {
            printf (" Child %d has no edgestr\n", ch);
        }
    }
    return children[ch];
#endif
}

/*
 * stree_get_num_children
 *
 * Return the number of children of a node.
 *
 * Parameters:  tree  -  a suffix tree
 *              node  -  a tree node
 *
 * Returns:  the number of children.
 */
int stree_get_num_children(SUFFIX_TREE tree, STREE_NODE node)
{
#ifdef PETER_GLOBAL
    return pglob_get_number_children(node->_index);
#else
    int i, count;
    STREE_NODE *children;

    if (int_stree_isaleaf(tree, node) || node->children == NULL)
        return 0;

    count = 0;
    children = (STREE_NODE *)node->children;
    for (i = 0; i < ALPHABET_SIZE; i++) {
        if (children[i] != NULL)
            count++;
    }
    return count;
#endif
}

/*
 * stree_get_children
 *
 * Returns a linked list of the children of a suffix tree node.
 *
 * Parameters:  tree  -  a suffix tree
 *              node  -  a tree node
 *
 * Returns: the head of the list of children;
 */
STREE_NODE stree_get_children(SUFFIX_TREE tree, STREE_NODE node)
{
    if (int_stree_isaleaf(tree, node) || !node_has_children(node)) {
        //printf(" isaleaf=%d, has_children=%d\n", int_stree_isaleaf(tree, node), node_has_children(node));
        return NULL;
    }

#ifdef PETER_GLOBAL    
    NODE_LIST children = pglob_get_children_list(node->_index);
 
    STREE_NODE head = NULL, tail = NULL;
    NODE_LIST::iterator child;
    for (child = children.begin(); child != children.end(); child++) {
        if (head == NULL)
            head = tail = *child;
        else
            tail = tail->next = *child;
    }
    if (tail) {
        tail->next = NULL;
    }

    return head;
#else
    int i;
    STREE_NODE head, tail, *children;
  
    head = tail = NULL;
    children = (STREE_NODE *)node->children;
    for (i = 0; i < ALPHABET_SIZE; i++) {
        if (children[i] != NULL) {
            if (head == NULL)
                head = tail = children[i];
            else
                tail = tail->next = children[i];
        }
    }
    if (tail) {
        tail->next = NULL;
    }

    return head;
#endif
}

/*
 * stree_get_labellen
 *
 * Get the length of the string labelling the path from the root to
 * a tree node.
 *
 * Parameters:  tree  -  a suffix tree
 *              node  -  a tree node
 *
 * Returns:  the length of the node's label.
 */
int stree_get_labellen(SUFFIX_TREE tree, STREE_NODE node)
{
    int len;
  
    len = 0;
    while (node != stree_get_root(tree)) {
        len += stree_get_edgelen(tree, node);
        node = stree_get_parent(tree, node);
    }
    return len;
}

/*
 * stree_get_label
 *
 * Get the string labelling the path from the root to a tree node and
 * store that string (or a part of the string) in the given buffer.
 *
 * If the node's label is longer than the buffer, then `buflen'
 * characters from either the beginning or end of the label (depending
 * on the value of `endflag') are copied into the buffer and the string
 * is NOT NULL-terminated.  Otherwise, the string will be NULL-terminated.
 *
 * Parameters:  tree     -  a suffix tree
 *              node     -  a tree node
 *              buffer   -  the character buffer
 *              buflen   -  the buffer length
 *              endflag  -  copy from the end of the label?
 */
void stree_get_label(SUFFIX_TREE tree, STREE_NODE node, CHAR_TYPE *buffer, int buflen, int endflag)
{
    int len, skip, edgelen;
    CHAR_TYPE *edgestr, *bufptr;

    len = stree_get_labellen(tree, node);
    skip = 0;

    if (buflen > len)
        buffer[len] = '\0';
    else {
        if (len > buflen && !endflag)
            skip = len - buflen;
        len = buflen;
    }

  /*
   * Fill in the buffer from the end to the beginning, as we move up
   * the tree.  If `endflag' is false and the buffer is smaller than
   * the label, then skip past the "last" `len - buflen' characters (i.e., the
   * last characters on the path to the node, but the first characters
   * that will be seen moving up to the root).
   */
  bufptr = buffer + len;
  while (len > 0 && node != stree_get_root(tree)) {
    edgelen = stree_get_edgelen(tree, node);

    if (skip >= edgelen)
      skip -= edgelen;
    else {
      if (skip > 0) {
        edgelen -= skip;
        skip = 0;
      }
      edgestr = stree_get_edgestr(tree, node) + edgelen;
      for ( ; len > 0 && edgelen > 0; edgelen--,len--)
        *--bufptr = *--edgestr;
    }

    node = stree_get_parent(tree, node);
  }
}

/*
 * stree_get_num_leaves
 *
 * Return the number of suffices that end at the tree node (these are the
 * "leaves" in the theoretical suffix tree).
 *
 * Parameters:  tree  -  a suffix tree
 *              node  -  a tree node
 *
 * Returns:  the number of "leaves" at that node.
 */
int stree_get_num_leaves(SUFFIX_TREE tree, STREE_NODE node)
{
    int i;
    STREE_INTLEAF intleaf;

    if (int_stree_isaleaf(tree, node)) {
        return 1;
    }
  
    intleaf = int_stree_get_intleaves(tree, node);
    for (i = 0; intleaf != NULL; i++) {
        intleaf = intleaf->next;
    }
    return i;
}

/*
 * stree_get_leaf
 *
 * Get the sequence information about one of the suffices that end at
 * a tree node.  The 'leafnum' parameter gives a number between 1 and
 * the number of "leaves" at the node, and information about that "leaf"
 * is returned.
 *
 * NOTE:  There is no ordering of the "leaves" at a node, either by
 *        string identifier number or by position.  You get whatever
 *        order the construction algorithm adds them to the node.
 *
 * Parameters:  tree        -  a suffix tree
 *              node        -  a tree node
 *              leafnum     -  which leaf to return
 *              string_out  -  address where to store the suffix pointer
 *                                (the value stored there points to the 
 *                                 beginning of the sequence containing the
 *                                 suffix, not the beginning of the suffix)
 *              pos_out     -  address where to store the position of the
 *                             suffix in the sequence
 *                                (a value between 1 and the sequence length)
 *              id_out      -  address where to store the seq. identifier
 *
 * Returns:  non-zero if a leaf was returned (i.e., the 'leafnum' value
 *           referred to a valid leaf), and zero otherwise.
 *           NOTE: If no leaf is returned, *string_out, *pos_out and *id_out
 *                 are left untouched.
 */    
int stree_get_leaf(SUFFIX_TREE tree, STREE_NODE node, int leafnum,
                   CHAR_TYPE **string_out, int *pos_out, int *id_out)
{
    if (int_stree_isaleaf(tree, node)) {
        if (leafnum != 1) {
            return 0;
        }
        STREE_LEAF leaf = (STREE_LEAF)node;
        *string_out = int_stree_get_string(tree, leaf->strid);
        *pos_out = leaf->pos;
        *id_out = int_stree_get_strid(tree, leaf->strid);
        return 1;
    } else {
        STREE_INTLEAF intleaf = int_stree_get_intleaves(tree, node);
        for (int i = 1; intleaf != NULL; i++,intleaf=intleaf->next) {
            if (i == leafnum) {
                *string_out = int_stree_get_string(tree, intleaf->strid);
                *pos_out = intleaf->pos;
                *id_out = int_stree_get_strid(tree, intleaf->strid);
                return 1;
            }
        }
        return 0;
    }
}

void stree_reset_stats(SUFFIX_TREE tree)
{
    tree->num_compares = tree->edges_traversed = tree->links_traversed = 0;
    tree->child_cost = tree->nodes_created = tree->creation_cost = 0;
}

/*
 *
 * Internal procedures to use when building and manipulating trees.
 *
 */

/*
 * int_stree_insert_string
 *
 * Insert a string into the list of strings maintained in the 
 * SUFFIX_TREE structure, in preparation for adding the suffixes of
 * the string to the tree.
 *
 * Parameters:  tree  -  A suffix tree
 *              S     -  The sequence
 *              M     -  The sequence length
 *
 * Returns:  The internal index into the SUFFIX_TREE structure's
 *           strings/lengths/ids arrays, or -1 on an error.
 */
int int_stree_insert_string(SUFFIX_TREE tree, CHAR_TYPE *S,  int M, int strid)
{
    int i, slot, newsize;

    if (tree->nextslot == tree->strsize) {
        if (tree->strsize == 0) {
            tree->strsize = 128;
            if ((tree->strings = (CHAR_TYPE **)my_malloc(tree->strsize * sizeof(char *))) == NULL)
                return -1;
            if ((tree->lengths = (int *)my_malloc(tree->strsize * sizeof(int))) == NULL 
                || (tree->ids = (int *)my_malloc(tree->strsize * sizeof(int))) == NULL)
                return -1;
            for (i = 0; i < 128; i++) {
                tree->strings[i] = NULL;
                tree->lengths[i] = tree->ids[i] = 0;
            }
        } else {
            newsize = tree->strsize + tree->strsize;
            if ((tree->strings = (CHAR_TYPE **)realloc(tree->strings, tree->strsize * sizeof(char *))) == NULL)
                return -1;
            if ((tree->lengths = (int *)realloc(tree->lengths, tree->strsize * sizeof(int))) == NULL 
                || (tree->ids = (int *)realloc(tree->ids,  tree->strsize * sizeof(int))) == NULL)
                return -1;
            for (i=tree->strsize; i < newsize; i++) {
                tree->strings[i] = NULL;
                tree->lengths[i] = tree->ids[i] = 0;
            }
            tree->strsize = newsize;
        }
    }

    slot = tree->nextslot;
    tree->strings[slot] = S;
    tree->lengths[slot] = M;
    tree->ids[slot] = strid;

    for (i = slot+1; i < tree->strsize; i++) {
        if (tree->strings[i] == NULL)
            break;
    }
    tree->nextslot = i;

    return slot;
}

/*
 * int_stree_delete_string
 *
 * Remove a string from the list of strings maintained in the
 * SUFFIX_TREE structure, as the last step in removing the
 * suffixes of that string from the tree.
 *
 * Parameters:  tree  -  A suffix tree
 *              id    -  The internal id for the string
 *
 * Returns: nothing
 */
void int_stree_delete_string(SUFFIX_TREE tree, int id)
{
    if (tree->strings[id] == NULL)
        return;

    if (tree->copyflag) {
        free(tree->strings[id]);
    }

    tree->strings[id] = NULL;
    if (id < tree->nextslot)
        tree->nextslot = id;
}

/*
 * int_stree_convert_leafnode
 *
 * Convert a LEAF structure into a NODE structure and replace the
 * NODE for the LEAF in the suffix tree..
 *
 * Parameters:  tree  -  a suffix tree
 *              node  -  a leaf of the tree
 *
 * Returns:  The NODE structure corresponding to the leaf, or NULL.
 */
STREE_NODE int_stree_convert_leafnode(SUFFIX_TREE tree, STREE_NODE node)
{
    STREE_NODE newnode;
    STREE_LEAF leaf;
    STREE_INTLEAF ileaf;

    leaf = (STREE_LEAF)node;

    newnode = int_stree_new_node(tree, leaf->edgestr, leaf->edgelen);
    if (newnode == NULL)
        return NULL;

    if ((ileaf = int_stree_new_intleaf(tree, leaf->strid, leaf->pos)) == NULL) {
        int_stree_free_node(tree, newnode);
        return NULL;
    }

    newnode->id = leaf->id;
    newnode->leaves = ileaf;

    int_stree_reconnect(tree, node->parent, node, newnode);
    int_stree_free_leaf(tree, leaf);

    return newnode;
}

/*
 * int_stree_get_suffix_link
 *
 * Traverses the suffix link from a node, and returns the node at the
 * end of the suffix link.
 *
 * Parameters:  tree  -  a suffix tree
 *              node  -  a tree node
 *
 * Return:  The node at the end of the suffix line.
 */
STREE_NODE int_stree_get_suffix_link(SUFFIX_TREE tree, STREE_NODE node)
{
    int len, edgelen;
    CHAR_TYPE *edgestr;
    STREE_NODE parent;

    if (node == stree_get_root(tree))
        return NULL;
    else if (!int_stree_isaleaf(tree, node))
        return node->suffix_link;

    edgestr = stree_get_edgestr(tree, node);
    edgelen = stree_get_edgelen(tree, node);
    parent = stree_get_parent(tree, node);

  /*
   * Do the skip/count trip to skip down to the proper node.
   */
    if (parent != stree_get_root(tree))
        parent = parent->suffix_link;
    else {
        edgestr++;
        edgelen--;
    }
    IF_STATS(_int64  temp = tree->child_cost);

    node = parent;
    while (edgelen > 0) {
        node = stree_find_child(tree, node, *edgestr);
        assert(node != NULL);

        len = stree_get_edgelen(tree, node);
        edgestr += len;
        edgelen -= len;
    }
    IF_STATS(tree->child_cost = temp);
    return node;
}

/*
 * int_stree_connect
 *
 * Connect a node as the child of another node.
 *
 * Parameters:  tree   -  A suffix tree
 *              parent -  The node to get the new child.
 *              child  -  The child being replaced
 *
 * Returns:  The parent after the child has been connected (if the
 *           parent was originally a leaf, this may mean replacing
 *           the leaf with a node).
 */
STREE_NODE int_stree_connect(SUFFIX_TREE tree, STREE_NODE parent, STREE_NODE child)
{
    if (int_stree_isaleaf(tree, parent) && (parent = int_stree_convert_leafnode(tree, parent)) == NULL)
        return NULL;

    child->parent = parent;
    CHAR_TYPE ch = stree_getch(tree, child);
    IF_STATS(tree->creation_cost++);


#ifdef PETER_GLOBAL
    pglob_set_child_node(parent->_index, ch, child->_index);
#else
    STREE_NODE *children = (STREE_NODE *)parent->children;
    children[(int) ch] = child;
#endif
    tree->idents_dirty = 1;

    return parent;
}

/*
 * int_stree_reconnect
 *
 * Replaces one node with another in the suffix tree, reconnecting
 * the link from the parent to the new node.
 *
 * Parameters:  tree      -  A suffix tree
 *              parent    -  The parent of the node being replaced
 *              oldchild  -  The child being replaced
 *              newchild  -  The new child
 *
 * Returns:  nothing
 */
void int_stree_reconnect(SUFFIX_TREE tree, STREE_NODE parent, STREE_NODE oldchild, STREE_NODE newchild)
{
#ifdef PETER_GLOBAL
    pglob_set_child_node(parent->_index, stree_getch(tree, newchild), newchild->_index);
#else
    STREE_NODE *children = (STREE_NODE *) parent->children;
    children[(int) stree_getch(tree, newchild)] = newchild;
#endif  

    newchild->parent = parent;
    oldchild->parent = NULL;

    tree->idents_dirty = 1;
}

/*
 * int_stree_disc_from_parent
 *
 * Disconnect a node from its parent in the tree.
 * NOTE:  This procedure only does the link manipulation part of the
 *        disconnection process.  int_stree_disconnect is the real
 *        disconnection function.
 *
 * Parameters:  tree    -  A suffix tree
 *              parent  -  The parent node
 *              child   -  The child to be disconnected
 *
 * Return:  nothing.
 */
void int_stree_disc_from_parent(SUFFIX_TREE tree, STREE_NODE parent, STREE_NODE child)
{
#ifdef PETER_GLOBAL
    pglob_disconnect_child_node(parent->_index, stree_getch(tree, child));
#else
    STREE_NODE *children = (STREE_NODE *) parent->children;
    children[(int) stree_getch(tree, child)] = NULL;
#endif
}

/*
 * int_stree_disconnect
 *
 * Disconnects a node from its parent, and compacts the tree if that
 * parent is no longer needed.
 *
 * Parameters:  tree  -  a suffix tree
 *              node  -  a tree node
 *
 * Return:  The node at the end of the suffix line.
 */
void int_stree_disconnect(SUFFIX_TREE tree, STREE_NODE node)
{
  int num;
  STREE_NODE parent;

  if (node == stree_get_root(tree))
    return;

  parent = stree_get_parent(tree, node);
  int_stree_disc_from_parent(tree, parent, node);

  if (parent->leaves == NULL && parent != stree_get_root(tree) && 
      (num = stree_get_num_children(tree, parent)) < 2) {
    if (num == 0) {
      int_stree_disconnect(tree, parent);
      int_stree_delete_subtree(tree, parent);
    }
    else if (num == 1)
      int_stree_edge_merge(tree, parent);
  }

  tree->idents_dirty = 1;
}

/*
 * int_stree_edge_split
 *
 * Splits an edge of the suffix tree, and adds a new node between two
 * existing nodes at that split point.
 *
 * Parameters:  tree  -  a suffix tree
 *              node  -  The tree node just below the split.
 *              len   -  How far down node's edge label the split is.
 *
 * Return:  The new node added at the split.
 */
STREE_NODE int_stree_edge_split(SUFFIX_TREE tree, STREE_NODE node, int len)
{
    STREE_NODE newnode, parent;

    // Assertions ? !@#$
    assert(node != stree_get_root(tree) && len > 0 && stree_get_edgelen(tree, node) > len);
    if (node == stree_get_root(tree) || len == 0 || stree_get_edgelen(tree, node) <= len)
        return NULL;

    newnode = int_stree_new_node(tree, node->edgestr, len);
    if (newnode == NULL)
        return NULL;

    parent = stree_get_parent(tree, node);
    int_stree_reconnect(tree, parent, node, newnode);
  
    node->edgestr += len;
    node->edgelen -= len;

    if (int_stree_connect(tree, newnode, node) == NULL) {
        node->edgestr -= len;
        node->edgelen += len;
        int_stree_reconnect(tree, parent, newnode, node);
        int_stree_free_node(tree, newnode);
        return NULL;
    }

    tree->num_nodes++;
    tree->idents_dirty = 1;

    return newnode;
}

/*
 * int_stree_edge_merge
 *
 * When a node has no "leaves" and only one child, this function will
 * remove that node and merge the edges from parent to node and node
 * to child into a single edge from parent to child.
 *
 * Parameters:  tree  -  A suffix tree
 *              node  -  The tree node to be removed
 *
 * Return:  nothing.
 */
void int_stree_edge_merge(SUFFIX_TREE tree, STREE_NODE node)
{
    int  len;
    STREE_NODE parent, child;

    if (node == stree_get_root(tree) || int_stree_isaleaf(tree, node) || node->leaves != NULL) {
        return;
    }
  
    parent = stree_get_parent(tree, node);
   
#if PETER_GLOBAL
    if (pglob_get_number_children(node->_index) != 1) {
        return;
    }
    child = pglob_get_children_list(node->_index).front();
#else
    child = NULL;
    STREE_NODE *children = (STREE_NODE *)node->children;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (children[i] != NULL) {
            if (child != NULL)
                return;
             child = children[i];
        }
    }
    if (child == NULL)
        return;
#endif

    len = stree_get_edgelen(tree, node);
    child->edgestr -= len;
    child->edgelen += len;

    int_stree_reconnect(tree, parent, node, child);
    tree->num_nodes--;
    tree->idents_dirty = 1;

    int_stree_free_node(tree, node);
}

/*
 * int_stree_add_intleaf
 *
 * Connects an intleaf to a node.
 *
 * Parameters:  tree  -  A suffix tree
 *              node  -  A tree node
 *              id    -  The internal identifier of the string
 *              pos   -  The position of the suffix in the string
 *
 * Returns:  Non-zero on success, zero on error.
 */
int int_stree_add_intleaf(SUFFIX_TREE tree, STREE_NODE node, int strid, int pos)
{
    STREE_INTLEAF intleaf;

    assert(!int_stree_isaleaf(tree, node)); // !@#$ Is this the intention?
    if (int_stree_isaleaf(tree, node))
        return 0;

    if ((intleaf = int_stree_new_intleaf(tree, strid, pos)) == NULL)
        return 0;

    intleaf->next = node->leaves;
    node->leaves = intleaf;
    return 1;
}

/*
 * int_stree_remove_intleaf
 *
 * Removes an intleaf from a node.
 *
 * Parameters:  tree  -  A suffix tree
 *              node  -  A tree node
 *              strid -  The internal identifier of the string
 *              pos   -  The position of the suffix in the string
 *
 * Returns:  Non-zero on success, zero on error.
 */
int int_stree_remove_intleaf(SUFFIX_TREE tree, STREE_NODE node,
                             int strid, int pos)
{
    STREE_INTLEAF intleaf, back;

    if (int_stree_isaleaf(tree, node) || !int_stree_has_intleaves(tree, node))
        return 0;

    back = NULL;
    intleaf = int_stree_get_intleaves(tree, node);
    for (back = NULL; intleaf != NULL; back=intleaf,intleaf=intleaf->next) {
        if (intleaf->strid == strid && intleaf->pos == pos)
            break;
    }

    if (intleaf == NULL)
        return 0;

    if (back != NULL)
        back->next = intleaf->next;
    else
        node->leaves = intleaf->next;

    int_stree_free_intleaf(tree, intleaf);
    return 1;
}

/*
 * int_stree_delete_subtree
 *
 * Free up all of the memory associated with the subtree rooted at node.
 *
 * Parameters:  tree  -  a suffix tree
 *              node  -  a tree node
 *
 * Return:  nothing.
 *
 * !@#$ Stack overflow on 80000 character strings. Replace with non-recursive version
 */
void int_stree_delete_subtree(SUFFIX_TREE tree, STREE_NODE node)
{
#ifdef PETER_NO_DELETE
    return;
#else
    int i;
    STREE_INTLEAF ileaf, itemp;

    if (int_stree_isaleaf(tree, node)) {
        int_stree_free_leaf(tree, (STREE_LEAF) node);
    } else {
        for (ileaf=node->leaves; ileaf != NULL; ileaf=itemp) {
            itemp = ileaf->next;
            int_stree_free_intleaf(tree, ileaf);
        }
#ifdef PETER_GLOBAL
        map<CHAR_TYPE, stree_node *> children = pglob_get_children_map(node->_index);
#else
        STREE_NODE  *children = (STREE_NODE *) node->children;
#endif
        for (i = 0; i < ALPHABET_SIZE; i++) {
            if (children[i] != NULL)
                int_stree_delete_subtree(tree, children[i]);
        }
        int_stree_free_node(tree, node);
    }
#endif // #ifdef PETER_NO_DELETE
}

/*
 * int_stree_walk_to_leaf
 *
 * Traverses the suffix link from a node, and returns the node at the
 * end of the suffix link.
 *
 * Parameters:  tree     -  A suffix tree
 *              node     -  The starting node of the walk
 *              pos      -  The starting point on the node's edge.
 *              T        -  The string to match
 *              N        -  The matching string's length
 *              node_out - Where the walk ends
 *              pos_out  - Where on the node's edge does the walk end.
 *
 * Return:  The number of characters matched during the walk.
 */
int int_stree_walk_to_leaf(SUFFIX_TREE tree, STREE_NODE node, int pos,
                           CHAR_TYPE *T, int N, STREE_NODE *node_out, int *pos_out)
{
    int len, edgelen;
    CHAR_TYPE *edgestr;
    STREE_NODE child;

    if (int_stree_isaleaf(tree, node)) {
        *node_out = node;
        *pos_out = pos;
        return 0;
    }

    edgestr = stree_get_edgestr(tree, node);
    edgelen = stree_get_edgelen(tree, node);
    len = 0;
    while (true) {
        while (len < N && pos < edgelen && T[len] == edgestr[pos]) {
            IF_STATS(tree->num_compares++);
            pos++;
            len++;
        }
        
        IF_STATS(tree->num_compares++);;
        if (len == N || pos < edgelen || (child = stree_find_child(tree, node, T[len])) == NULL)
            break;
        
        IF_STATS(tree->edges_traversed++);
        if (int_stree_isaleaf(tree, child)) {
            *node_out = child;
            *pos_out = 0;
            return len;
        }

        node = child;
        edgestr = stree_get_edgestr(tree, node);
        edgelen = stree_get_edgelen(tree, node);
        pos = 1;
        len++;
    }

    *node_out = node;
    *pos_out = pos;
    return len;
}

/*
 * int_stree_set_idents
 *
 * Uses the non-recursive traversal to set the identifiers for the current
 * nodes of the suffix tree.  The nodes are numbered in a depth-first
 * manner, beginning from the root and taking the nodes in the order they
 * appear in the children lists.
 *
 * Parameters:  tree  -  A suffix tree
 *
 * Return:  nothing.
 */
#if 0 // HACK !@#$
void int_stree_set_idents(SUFFIX_TREE tree) {}
#else
void int_stree_set_idents(SUFFIX_TREE tree)
{
    enum { START, FIRST, MIDDLE, DONE, DONELEAF } state;
    int i, num, childnum, nextid;
    STREE_NODE root, node, child;

    if (!tree->idents_dirty)
        return;

   /*
    * Use a non-recursive traversal where the 'isaleaf' field of each node
    * is used as the value remembering the child currently being
    * traversed.
    */
    nextid = 0;
    node = root = stree_get_root(tree);
    state = START;
    while (true) {
        // The first time we get to a node.
        if (state == START) {
            node->id = nextid++;
            num = stree_get_num_children(tree, node);
            state = (num > 0) ? FIRST: DONELEAF;
        }

        // Start or continue recursing on the children of the node.
        if (state == FIRST || state == MIDDLE) {
            
            // Look for the next child to traverse down to.
            childnum = (state == FIRST) ? 0 : node->isaleaf;
#ifdef PETER_GLOBAL
            map<CHAR_TYPE, stree_node *> children = pglob_get_children_map(node->_index);
            //pglob_dump_map(children, "children");
            CHAR_LIST keys = pglob_get_children_keys(node->_index);
            i = childnum;
            child = (childnum < (int)keys.size()) ? children[keys[childnum]] : NULL;
            //cout << "num children=" << keys.size() << ", childnum=" << childnum;
            //if (childnum < (int)keys.size())
            //    cout << ",val=" << (int)keys[childnum]; 
            //else
            //    cout << "<end>" ;
            //cout << endl;
#else
            STREE_NODE *children = (STREE_NODE *)node->children;
            for (i = childnum; i < ALPHABET_SIZE; i++) {
                if (children[i] != NULL)
                    break;
            }
            child = (i < ALPHABET_SIZE) ? children[i] : NULL;
#endif    
            if (child == NULL) {
                state = DONE;
            } else {
                node->isaleaf = i + 1;
                node = child;
                state = START;
            }
        }

       /*
        * Last time we get to a node, do the post-processing and move back up,
        * unless we're at the root of the traversal, in which case we stop.
        */
        if (state == DONE || state == DONELEAF) {
            if (state == DONE)
                 node->isaleaf = 0;

            if (node == root)
                break;
            
            node = node->parent;
            state = MIDDLE;
        }
    }

    tree->idents_dirty = 0;
}
#endif

/*
 * int_stree_new_intleaf
 *
 * Allocates memory for a new INTLEAF structure.
 *
 * Parameters:  tree   -  A suffix tree
 *              strid  -  The id of the string containing the new suffix
 *                        to be added to the tree.
 *              pos    -  The position of the new suffix in the string.
 *
 * Returns:  The structure or NULL.
 */
STREE_INTLEAF int_stree_new_intleaf(SUFFIX_TREE tree, int strid, int pos)
{
    STREE_INTLEAF ileaf;

    if (!(ileaf = (STREE_INTLEAF)my_calloc(sizeof(SINTLEAF_STRUCT), 1)))
        return NULL;

    ileaf->strid = strid;
    ileaf->pos = pos;

    IF_STATS(tree->tree_size += OPT_INTLEAF_SIZE);
    return ileaf;
}
  
/*
 * int_stree_new_leaf
 *
 * Allocates memory for a new LEAF structure.
 *
 * Parameters:  tree        -  A suffix tree
 *              strid       -  The id of the string containing the new suffix  to be added to the tree.
 *              edgepos     -  The position of the edge label in the string.
 *              leafpos     -  The position of the new suffix in the string.
 *
 * Returns:  The structure or NULL.
 */
STREE_LEAF int_stree_new_leaf(SUFFIX_TREE tree, int strid, int edgepos,
                              int leafpos)
{
    STREE_LEAF leaf;

    if ((leaf = (STREE_LEAF)my_calloc(sizeof(SLEAF_STRUCT), 1)) == NULL)
        return NULL;

    leaf->isaleaf = 1;
    leaf->strid = strid;
    leaf->pos = leafpos;
    leaf->edgestr = int_stree_get_string(tree, strid) + edgepos;
    leaf->edgelen = int_stree_get_length(tree, strid) - edgepos;

    IF_STATS(tree->tree_size += OPT_LEAF_SIZE);

#ifdef PETER_GLOBAL
    leaf->_index = pglob_get_node_index();
    pglob_add_node(leaf->_index, (stree_node*)leaf);
#endif

    return leaf;
}

/*
 * int_stree_new_node
 *
 * Allocates memory for a new NODE structure.
 *
 * Parameters:  tree        -  A suffix tree
 *              edgestr     -  The edge label on the edge to the node.
 *              edgelen     -  The edge label's length.
 *
 * Returns:  The structure or NULL.
 */
STREE_NODE int_stree_new_node(SUFFIX_TREE tree, CHAR_TYPE *edgestr, int edgelen)
{
    STREE_NODE node = (STREE_NODE)my_calloc(sizeof(SNODE_STRUCT), 1); 
    if (!node)  {
        return NULL;
    }
        
    node->edgestr = edgestr;
    node->edgelen = edgelen;
 
#ifndef PETER_GLOBAL
    node->children = (STREE_NODE)my_calloc(ALPHABET_SIZE * sizeof(STREE_NODE), 1);
    if (node->children == NULL) {
        free(node);
        return NULL;
    }
#else
    node->_index = pglob_get_node_index();
    pglob_add_node(node->_index, node);
#endif
    IF_STATS(tree->tree_size += ALPHABET_SIZE * sizeof(STREE_NODE) + OPT_NODE_SIZE);
    return node;
}

/*
 * int_stree_free_{intleaf,leaf,node}
 *
 * Free the memory used for an INTLEAF, LEAF or NODE structure.  Also,
 * if the NODE structure uses a children array, free that space too.
 *
 * Parameters:  ileaf/leaf/node  -  The structure to free.
 *
 * Return:  nothing.
 */
void int_stree_free_intleaf(SUFFIX_TREE tree, STREE_INTLEAF ileaf)
{
    IF_STATS(tree->tree_size -= OPT_INTLEAF_SIZE);
    free(ileaf);
}

void int_stree_free_leaf(SUFFIX_TREE tree, STREE_LEAF leaf)
{
    IF_STATS(tree->tree_size -= OPT_LEAF_SIZE);
    free(leaf);
}

void int_stree_free_node(SUFFIX_TREE tree, STREE_NODE node)
{
    IF_STATS(tree->tree_size -= ALPHABET_SIZE * sizeof(STREE_NODE) + OPT_NODE_SIZE);
#ifdef PETER_GLOBAL
    pglob_delete_node(node->_index);
#else
    free(node->children);
#endif
    free(node);
}



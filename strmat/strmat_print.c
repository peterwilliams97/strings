
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "strmat.h"
#include "stree_strmat.h"
#include "strmat_alpha.h"
#include "strmat_print.h"

/*
 * Procedures for printing a suffix tree.
 */

int large_print_tree(SUFFIX_TREE tree, STREE_NODE node, int gen_stree_flag)
{
    int j, pos, edgelen, leafnum, index;
    char *edgestr, buffer[34];
    STREE_NODE child;
    char *str;

    if (node == stree_get_root(tree))
        mprintf("   Node %d:    (Root)", stree_get_ident(tree, node));
    else
        mprintf("   Node %d:    (Link to %d)", stree_get_ident(tree, node),
            stree_get_ident(tree, stree_get_suffix_link(tree, node)));

    leafnum = 1;
    while (stree_get_leaf(tree, node, leafnum, &str, &pos, &index)) {
        if (leafnum == 1)
            mprintf("   (Leaf ");
        else
            mprintf(", ");

        if (gen_stree_flag)
            mprintf("%d:%d", index, pos + 1);
        else
            mprintf("%d", pos + 1);

        leafnum++;
    }
    if (leafnum > 1)
        mprintf(")\n");
    else
        mputc('\n');

    child = stree_get_children(tree, node);
    while (child != NULL) {
        edgestr = stree_get_rawedgestr(tree, child);
        edgelen = stree_get_edgelen(tree, child);

        for (j=0; j < 30 && j < edgelen; j++)
            buffer[j] = (isprint((int)edgestr[j]) ? edgestr[j] : '#');
        buffer[j] = '\0';

        if (edgelen > 30) {
            buffer[30] = buffer[31] = buffer[32] = '.';
            buffer[33] = '\0';
        }

        mprintf("      %s  ->  Node %d", buffer, stree_get_ident(tree, child));

    leafnum = 1;
    while (stree_get_leaf(tree, child, leafnum, &str, &pos, &index)) {
        if (leafnum == 1)
            mprintf("   (Leaf #");
        else
            mprintf(", ");

        if (gen_stree_flag)
            mprintf("%d:%d", index, pos + 1);
        else
            mprintf("%d", pos + 1);

        leafnum++;
    }
    if (leafnum > 1)
        mputc(')');

    if (mputc('\n') == 0)
        return 0;

    child = stree_get_next(tree, child);
  }
  if (mputc('\n') == 0)
    return 0;

  child = stree_get_children(tree, node);
  while (child != NULL) {
    if (stree_get_num_children(tree, child) > 0)
      if (large_print_tree(tree, child, gen_stree_flag) == 0)
        return 0;

    child = stree_get_next(tree, child);
  }

  return 1;
}

int small_print_tree(SUFFIX_TREE tree, STREE_NODE node, int depth, int gen_stree_flag)
{
    char buffer[256], buf2[128];
    int i, index, pos, leafnum, edgelen, status;
    char *edgestr, *str;
    STREE_NODE child;

    memset(buffer, 0, sizeof(buffer));
    memset(buf2, 0, sizeof(buf2));

    if (node == stree_get_root(tree)) {
        mprintf("   Root %d:\n", stree_get_ident(tree, node));
        edgelen = 0;
    } else {
        mprintf("   Node %2d->%2d:  ",
            stree_get_ident(tree, stree_get_parent(tree, node)),
            stree_get_ident(tree, node));
    
        edgestr = stree_get_rawedgestr(tree, node);
        edgelen = stree_get_edgelen(tree, node);

        for (i = 0; i < depth; i++)
            buffer[i] = '.';
        strcpy(buffer + i, "'");
        i++;
        strncpy(buffer + i, edgestr, edgelen);
        i += edgelen;
        strcpy(buffer + i, "'");
        i++;
        while (i < 20) {
            buffer[i++] = ' ';
        }
        buffer[i] = '\0';
        //mputs(buffer);

        leafnum = 1;
        while (stree_get_leaf(tree, node, leafnum, &str, &pos, &index)) {
            if (leafnum == 1) {
                //mputs(" $");
                strcpy(buffer + i, " $");
                i += 2;
            } else {
                //mputc(',');
                strcpy(buffer + i, ",");
                i += 1;
            }

            if (gen_stree_flag) {
                //mprintf("%d:%d", index, pos + 1);
                sprintf(buf2,"%d:%d", index, pos + 1);
                strcpy(buffer + i, buf2);
                i += strlen(buf2);
            }
            else {
                //mprintf("%d", pos + 1);
                sprintf(buf2,"%d", pos + 1);
                strcpy(buffer + i, buf2);
                i += strlen(buf2);
            }

            leafnum++;
        }

        while (i < 30) {
            buffer[i++] = ' ';
        }

        mputs(buffer);

        status = mprintf("     (Link %2d->%2d)\n", stree_get_ident(tree, node),
                         stree_get_ident(tree, stree_get_suffix_link(tree, node)));
        if (status == 0)
          return 0;
    }

    child = stree_get_children(tree, node);
    while (child != NULL) {
        if (small_print_tree(tree, child, depth + edgelen, gen_stree_flag) == 0)
            return 0;
        child = stree_get_next(tree, child);
    }

    return 1;
}

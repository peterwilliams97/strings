#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "strmat.h"
#include "stree_strmat.h"
#include "strmat_util.h"
#include "strmat_print.h"

/*
 * Procedures for printing a suffix tree.
 */

int large_print_tree(SUFFIX_TREE tree, STREE_NODE node, int gen_stree_flag)
{
    int j, pos, edgelen, leafnum, index;
    CHAR_TYPE *edgestr, buffer[34];
    STREE_NODE child;
    char *str;

    if (node == stree_get_root(tree))
        printf("   Node %d:    (Root)", stree_get_ident(tree, node));
    else
        printf("   Node %d:    (Link to %d)", stree_get_ident(tree, node),
            stree_get_ident(tree, stree_get_suffix_link(tree, node)));

    leafnum = 1;
    while (stree_get_leaf(tree, node, leafnum, &str, &pos, &index)) {
        if (leafnum == 1)
            printf("   (Leaf ");
        else
            printf(", ");

        if (gen_stree_flag)
            printf("%d:%d", index, pos + 1);
        else
            printf("%d", pos + 1);

        leafnum++;
    }
    if (leafnum > 1)
        printf(")\n");
    else
        mputc('\n');

    child = stree_get_children(tree, node);
    while (child != NULL) {
        edgestr = stree_get_edgestr(tree, child);
        edgelen = stree_get_edgelen(tree, child);

        for (j=0; j < 30 && j < edgelen; j++)
            buffer[j] = (isprint((int)edgestr[j]) ? edgestr[j] : '#');
        buffer[j] = '\0';

        if (edgelen > 30) {
            buffer[30] = buffer[31] = buffer[32] = '.';
            buffer[33] = '\0';
        }

        printf("      %s  ->  Node %d", buffer, stree_get_ident(tree, child));

        leafnum = 1;
        while (stree_get_leaf(tree, child, leafnum, &str, &pos, &index)) {
            if (leafnum == 1)
                printf("   (Leaf #");
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

        if (mputc('\n') == 0)
            return 0;

        child = stree_get_next(tree, child);
    }
    if (mputc('\n') == 0)
        return 0;

    child = stree_get_children(tree, node);
    while (child != NULL) {
        if (stree_get_num_children(tree, child) > 0) {
            if (large_print_tree(tree, child, gen_stree_flag) == 0)
            return 0;
        }
        child = stree_get_next(tree, child);
    }
    return 1;
}

#define COLUMN_1 50
#define COLUMN_2 80

void small_print_tree(SUFFIX_TREE tree, STREE_NODE node, int depth, int gen_stree_flag)
{
    char buffer[256], buf2[128];
    int i, index, pos, leafnum, edgelen, status;
    CHAR_TYPE *edgestr, *str;
    STREE_NODE child;

    if (depth * 3 > COLUMN_1 -10) {
        return;
    }

    memset(buffer, 0, sizeof(buffer));
    memset(buf2, 0, sizeof(buf2));

    if (node == stree_get_root(tree)) {
        printf("   Root %d:\n", stree_get_ident(tree, node));
        edgelen = 0;
    } else {
        printf("   Node %2d->%2d:  ",
            stree_get_ident(tree, stree_get_parent(tree, node)),
            stree_get_ident(tree, node));
    
        edgestr = stree_get_edgestr(tree, node);
        edgelen = stree_get_edgelen(tree, node);

        for (i = 0; i < depth*3; i++) {
            buffer[i] = '.';
        }
        {
            char char_buf[CHAR_BUFFER_LEN];
            int char_len;
            get_char_array(edgestr, edgelen, char_buf);
            char_len = strlen(char_buf);
            //strncpy(buffer + i, edgestr, edgelen);
            //i += edgelen;
            strncpy(buffer + i, char_buf, char_len);
            i += char_len;
        }
        while (i < COLUMN_1) {
            buffer[i++] = ' ';
        }
        buffer[i] = '\0';
        
        leafnum = 1;
        while (stree_get_leaf(tree, node, leafnum, &str, &pos, &index)) {
            if (i >= COLUMN_2 -3) {
                strcpy(buffer + i, "...");
                i += 3;
                break;
            }
            if (leafnum == 1) {
                strcpy(buffer + i, " $");
                i += 2;
            } else {
                strcpy(buffer + i, ",");
                i += 1;
            }

            if (gen_stree_flag) {
                sprintf(buf2,"%d:%d", index, pos + 1);
                strcpy(buffer + i, buf2);
                i += strlen(buf2);
            }
            else {
                sprintf(buf2,"%d", pos + 1);
                strcpy(buffer + i, buf2);
                i += strlen(buf2);
            }
            leafnum++;
        }

        while (i < COLUMN_2) {
            buffer[i++] = ' ';
        }

        mputs(buffer);

        status = printf("(Link %2d->%2d)\n", stree_get_ident(tree, node),
                         stree_get_ident(tree, stree_get_suffix_link(tree, node)));
    }

    child = stree_get_children(tree, node);
    while (child != NULL) {
        small_print_tree(tree, child, depth + edgelen, gen_stree_flag);
        child = stree_get_next(tree, child);
    }

}

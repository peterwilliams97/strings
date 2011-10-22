#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <algorithm>
#include "strmat.h"
#include "stree_strmat.h"
#include "strmat_util.h"
#include "strmat_print.h"

using namespace std;
#if 0
string get_label(const SUFFIX_TREE tree, const STREE_NODE z)
{
    string desc = "";
    if (z != stree_get_root(tree))  {
        int len = stree_get_labellen(tree, z);
        static const int BUFFER_SIZE = 64;
        CHAR_TYPE buffer[BUFFER_SIZE];
        stree_get_label(tree, z, buffer, BUFFER_SIZE, 0);
        char char_buf[CHAR_BUFFER_LEN];
        get_char_array(buffer, BUFFER_SIZE,char_buf);
        desc = char_buf;
    }

    return desc;
    //stringstream out;
    //out << stree_get_ident(tree, z) << " (" << desc << ")";
    //return out.str();
}
#else
string get_label(const SUFFIX_TREE tree, const STREE_NODE node)
{
    string out = node->parent ? get_label(tree, node->parent) : ""; 
    return out + str_to_string(node->edgestr, node->edgelen);
}
#endif

void print_label(const SUFFIX_TREE tree, const STREE_NODE node, string title)
{
    stringstream s;
    s << " <" << title << " id=" << node->id;
    string label = get_label(tree, node);
    s << " [" << label << "]  " << label.size()/3;
    s << ">";
    cout << setw(45) << left << s.str();
}

void print_node(const STREE_NODE node, const char *title)
{
    stringstream s;
    if (title) {
        s << " <" << title << ":" ;
    } else {
        s << "Node    " << endl;
    }
    s << "id=" << node->id;
#if defined(PETER_GLOBAL) && 0
    cout << " (index=" << node->_index << ")";
#endif
    char buffer[CHAR_BUFFER_LEN];
    s << " edge=[" << get_char_array(node->edgestr, node->edgelen, buffer) << "]  " << node->edgelen;
    if (!title) {
        s << endl;
    } else {
        s << ">";
    }
    cout << setw(40) << left << s.str();
}

/*
 * Procedures for printing a suffix tree.
 */
int large_print_tree(SUFFIX_TREE tree, STREE_NODE node, int gen_stree_flag)
{
    int j, pos, edgelen, leafnum, index;
    CHAR_TYPE *edgestr, buffer[34];
    STREE_NODE child;
    CHAR_TYPE *str;

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

#define COLUMN_1 60
#define COLUMN_2 90

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
        printf("   Node %4d->%4d:  ",
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
            char_len = min((int)strlen(char_buf), COLUMN_1 - 5);
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

    //if (stree_get_num_children(tree, node) > 1) {
    //    printf("\n -- %d children ---\n", stree_get_num_children(tree, node));
    //}

    child = stree_get_children(tree, node);
    while (child != NULL) {
        small_print_tree(tree, child, depth + edgelen, gen_stree_flag);
        child = stree_get_next(tree, child);
    }
}

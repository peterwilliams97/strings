#ifndef PETER_GLOBAL
// #error #ifdef PETER_GLOBAL must be defined
#endif

#ifndef _PETER_GLOBAL_H_
#define _PETER_GLOBAL_H_

#include <list>
#include <map>
#include "peter_common.h"

struct stree_node;

#ifdef PETER_GLOBAL 

// Initialization & clean-up
 void        pglob_init();
 void        pglob_clear();

// Node/leaf numbering
 int         pglob_get_node_index();

// Node/leaf tracking
 void        pglob_add_node(int, stree_node*);
 void        pglob_delete_node(int);
 stree_node *pglob_get_node(int index);

 // Child node/leaf tracking
 void        pglob_set_child_node(int index, CHAR_TYPE ch, int child_index);
 void        pglob_disconnect_child_node(int index, CHAR_TYPE ch);
 stree_node *pglob_get_child_node(int index, CHAR_TYPE ch);
 int         pglob_get_number_children(int index);
 std::list<stree_node *> 
            pglob_get_children_list(int index);
 std::map<CHAR_TYPE, stree_node *> 
            pglob_get_children_map(int index);

#endif

#endif // #define _PETER_GLOBAL_H_
#ifndef PETER_GLOBAL
// #error #ifdef PETER_GLOBAL must be defined
#endif

#ifndef _PETER_GLOBAL_H_
#define _PETER_GLOBAL_H_

#ifdef USE_LIST
 #include <list>
 #define LIST_TYPE list
#else
 #include <vector>
 #define LIST_TYPE vector
#endif
#include <map>
#include <string>
#include "peter_common.h"

struct stree_node;

#ifdef PETER_GLOBAL 

 typedef std::LIST_TYPE<CHAR_TYPE> CHAR_LIST; 
 typedef std::LIST_TYPE<stree_node *> NODE_LIST; 
 typedef std::map<CHAR_TYPE, stree_node *>  NODE_MAP;

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
 NODE_LIST   pglob_get_children_list(int index);
 NODE_MAP    pglob_get_children_map(int index);
 CHAR_LIST   pglob_get_children_keys(int index);

 template <class K, class V> 
 void pglob_dump_map(std::map<K,V> m, std::string desc)
 {
     cout << "pglob_dump_map: " << desc << endl;   
     for (std::map<K,V>::iterator it = m.begin(); it != m.end(); ++it) {
        cout << "  "<< (int)it->first << " : " << it->second << endl;
    }
 }

#endif

#endif // #define _PETER_GLOBAL_H_
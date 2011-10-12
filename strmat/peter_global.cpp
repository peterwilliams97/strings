#include <list>
#include <map>
#include "peter_global.h"

using namespace std;

#define COMPOUND_CHILD 0

#if COMPOUND_CHILD
struct NodeChar {
    int _node_index;
    CHAR_TYPE _ch;
    NodeChar(int node_index, CHAR_TYPE ch) { _node_index = node_index; _ch = ch; }
    _int64 get() { return _ch * (_int64)0x1000000 + _node_index; }
};
#endif

static int _node_index = 0;
static map<int, stree_node*> _node_dict;

#if COMPOUND_CHILD
 map<_int64, int> _child_node_dict;
#else
 static map<int, map<CHAR_TYPE, int>> _child_node_dict2;
#endif

void pglob_clear()
{
    _node_dict.clear();
    for (map<int, map<CHAR_TYPE, int>>::iterator it = _child_node_dict2.begin(); it != _child_node_dict2.end(); it++) {
        it->second.clear();
    }
    _child_node_dict2.clear();
}

void pglob_init()
{
    pglob_clear();
    
    _node_index = 0;
    _node_dict = map<int, stree_node*>(); 
    _child_node_dict2 = map<int, map<CHAR_TYPE, int>>();
}

int pglob_get_node_index()
{
    return _node_index++;
}

void pglob_add_node(int index, stree_node *node)
{
    _node_dict[index] = node;
}

void pglob_delete_node(int index)
{
    _node_dict.erase(index);
}

stree_node *pglob_get_node(int index)
{
    return _node_dict[index];
}

void pglob_set_child_node(int index, CHAR_TYPE ch, int child_index)
{
#if COMPOUND_CHILD
    NodeChar nc(index, ch);
    _child_node_dict[nc.get()] = child_index; 
#else
    if (_child_node_dict2.find(index) == _child_node_dict2.end()) {
        _child_node_dict2[index] = map<CHAR_TYPE, int>();
    }
    _child_node_dict2[index][ch] = child_index;
#endif
}

void pglob_disconnect_child_node(int index, CHAR_TYPE ch)
{
#if COMPOUND_CHILD
    NodeChar nc(index, ch);
    _child_node_dict.erase(nc.get()); 
#else
    _child_node_dict2[index].erase(ch);
    if (_child_node_dict2.find(index) == _child_node_dict2.end()) {
       _child_node_dict2.erase(index);
    }
#endif
}

stree_node *pglob_get_child_node(int index, CHAR_TYPE ch)
{
#if COMPOUND_CHILD
    NodeChar nc(index, ch);
    return _node_dict[_child_node_dict[nc.get()]]; 
#else
    if (_child_node_dict2.find(index) == _child_node_dict2.end()) {
        return 0;
    } 
    if (_child_node_dict2[index].find(ch) == _child_node_dict2[index].end()) {
        return 0;
    } 
    int child_index = _child_node_dict2[index][ch]; 
    return pglob_get_node(child_index);
#endif
}

int pglob_get_number_children(int index)
{
#if COMPOUND_CHILD
#else
    return _child_node_dict2[index].size();
#endif
}

LIST_TYPE<stree_node *> pglob_get_children_list(int index)
{
#if COMPOUND_CHILD
#else
    map<CHAR_TYPE, int> index_map = _child_node_dict2[index];
    LIST_TYPE<stree_node *> node_list  = LIST_TYPE<stree_node *>();

    for (map<CHAR_TYPE,int>::iterator it = index_map.begin(); it != index_map.end(); it++) {
        node_list.push_back(pglob_get_node(it->second));
    }
    return node_list;
#endif
}

map<CHAR_TYPE, stree_node *> pglob_get_children_map(int index)
{
#if COMPOUND_CHILD
#else
    map<CHAR_TYPE, int> index_map = _child_node_dict2[index];
    map<CHAR_TYPE, stree_node *> node_map = map<CHAR_TYPE, stree_node *>();

    for (map<CHAR_TYPE,int>::iterator it = index_map.begin(); it != index_map.end(); it++) {
        node_map[it->first] = pglob_get_node(it->second);
    }
    return node_map;
#endif
}

template <class K, class V> 
LIST_TYPE<K> get_keys(map<K,V> m)
{
    LIST_TYPE<K> keys;
    for (map<K,V>::iterator it = m.begin(); it != m.end(); ++it) {
        keys.push_back(it->first);
    }
    return keys;
}

LIST_TYPE<CHAR_TYPE> pglob_get_children_keys(int index)
{
    map<CHAR_TYPE, stree_node *> children = pglob_get_children_map(index);
    return get_keys(children);
    //vector<CHAR_TYPE> keys;
    //for (map<CHAR_TYPE, stree_node *>::iterator it = children.begin(); it != children.end(); ++it) {
    //    keys.push_back(it->first);
    //}
    //return keys;
}
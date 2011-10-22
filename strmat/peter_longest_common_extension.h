#ifndef _PETER_LONGEST_COMMON_EXTENSIONL_H_
#define _PETER_LONGEST_COMMON_EXTENSIONL_H_

struct LCA_STRUCT;
struct STRING;
//struct STREE_NODE;
struct stree_node;

struct LCE { 
    LCA_STRUCT *_lca;
    stree_node **_nodemap;
    STRING *_s1;
    STRING *_s2;
};

LCE *prepare_longest_common_extension(STRING *s1, STRING *s2, bool print_stats);
void longest_common_extension_free(LCE *lce);
/*
 * Return longest common extension of
 *      offset ofs1 into string lce->s1
 *      offset ofs2 into string lce->s2
 */
STREE_NODE lookup_lce(LCE *lce, int ofs1, int ofs2);


#endif // #define _PETER_LONGEST_COMMON_EXTENSIONL_H_
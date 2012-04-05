
bool strmat_ukkonen_build(const STRING **strings, int num_strings, bool print_stats, bool print_tree);
//int strmat_weiner_build(STRING **strings, int num_strings, int build_policy, int build_threshold, int print_stats, int print_tree);
bool strmat_stree_match(const STRING *pattern, const STRING **strings, int num_strings, bool print_stats, bool print_matches);
bool strmat_stree_walkaround(STRING **strings, int num_strings);
bool strmat_stree_naive_lca(const STRING **strings, int num_strings, bool print_stats);
bool strmat_stree_lca(const STRING **strings, int num_strings, bool print_stats);
//int strmat_stree_lempel_ziv(STRING *text, int build_policy, int build_threshold, int stats_flag, char mode);


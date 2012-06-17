#ifndef INVERTED_INDEX_H
#define INVERTED_INDEX_H

#include <string>
#include <vector>

struct InvertedIndex;

InvertedIndex *create_inverted_index(const std::vector<std::string> &filenames);
void show_inverted_index(const std::string title, const InvertedIndex *inverted_index);
std::vector<std::string> get_all_repeats(InvertedIndex *inverted_index);

#endif // #ifndef INVERTED_INDEX_H

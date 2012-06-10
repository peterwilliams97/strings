// Specify number of times a term must occur in a doc
struct Occurrence {
    int doc_index;          // Index into docs
    unsigned int num;       // Number of occurrences
    Occurrence(int index, unsigned int n) {
        doc_index = index;
        num = n;
    }
};

struct InvertedIndex;

InvertedIndex *make_inverted_index(const std::vector<std::string> filenames);
void show_inverted_index(const std::string title, const InvertedIndex *inverted_index);
int get_doc_index(const InvertedIndex *inverted_index, std::string doc);
std::vector<std::string> get_all_repeats(InvertedIndex *inverted_index, const std::vector<Occurrence> occurrences);


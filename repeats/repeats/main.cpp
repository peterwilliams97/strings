#include <fstream>
#include <iostream>
#include "utils.h"
#include "inverted_index.h"

using namespace std;

struct TestEntry {
    unsigned int num_repeats;
    string filename;
};

/*
 * Test data made with
 *  https://github.com/peterwilliams97/strings/blob/master/make_repeats/make_repeats.py
 */
TestEntry entries[] = {
  { 11, string("C:\\dev\\suffix\\make_repeats\\repeats=11.txt") },
  { 12, string("C:\\dev\\suffix\\make_repeats\\repeats=12.txt") },
  { 13, string("C:\\dev\\suffix\\make_repeats\\repeats=13.txt") },
  { 14, string("C:\\dev\\suffix\\make_repeats\\repeats=14.txt") },
  { 15, string("C:\\dev\\suffix\\make_repeats\\repeats=15.txt") },
  { 16, string("C:\\dev\\suffix\\make_repeats\\repeats=16.txt") },
  { 17, string("C:\\dev\\suffix\\make_repeats\\repeats=17.txt") },
  { 18, string("C:\\dev\\suffix\\make_repeats\\repeats=18.txt") },
  { 19, string("C:\\dev\\suffix\\make_repeats\\repeats=19.txt") },
  { 20, string("C:\\dev\\suffix\\make_repeats\\repeats=20.txt") }
};
const int NUM_TEST_FILES = NUMELEMS(entries);

static vector<string> 
get_filenames() {
    list<string> filenames;
    for (int i = 0; i < NUM_TEST_FILES; i++) {
        filenames.push_back(entries[i].filename);
    }
    return vector<string>(filenames.begin(), filenames.end());
}

void 
test() {
    vector<string> filenames = get_filenames();

    InvertedIndex *inverted_index = create_inverted_index(filenames);
    cout << "========================================================" << endl;
    
    show_inverted_index("initial", inverted_index);
    cout << "========================================================" << endl;
    
    vector<string> repeats = get_all_repeats(inverted_index);
    cout << "========================================================" << endl;
    
    cout << "Found " << repeats.size() << " repeated strings";
    if (repeats.size() > 0) {
        cout << " of length " << repeats.front().size();
    }
    cout << endl;
    print_vector("Repeated strings", repeats);
}

int main() {
    test();
    return 0;
}


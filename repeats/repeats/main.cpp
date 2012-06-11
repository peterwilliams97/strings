#include <map>
#include <vector>
#include <list>
#include <string>
#include <fstream>
#include <iostream>
#include "utils.h"
#include "inverted_index.h"

// for lower_bound/upper_bound example
#include <algorithm>


using namespace std;

struct TestEntry {
    unsigned int num_repeats;
    string filename;
};

const int NUM_TEST_FILES = 2;
TestEntry entries[NUM_TEST_FILES] = {
    {  1, string("C:\\dev\\suffix\\make_repeats\\repeats=1.txt") },
    {  2, string("C:\\dev\\suffix\\make_repeats\\repeats=2.txt") }
};

static vector<string> get_filenames() {
    list<string> filenames;
    for (int i = 0; i < NUM_TEST_FILES; i++) {
        filenames.push_back(entries[i].filename);
    }
    return vector<string>(filenames.begin(), filenames.end());
}

void test() {
   
    vector<string> filenames = get_filenames();
    InvertedIndex *inverted_index = make_inverted_index(filenames);
    show_inverted_index("initial", inverted_index);

    list<Occurrence> occurrences;
    for (int i = 0; i < NUM_TEST_FILES; i++)  {
        int doc_index = get_doc_index(inverted_index, entries[i].filename);
        if (doc_index >= 0) {
            occurrences.push_back(Occurrence(doc_index, entries[i].num_repeats));
        }
    }
    cout << "Num occcurrences = " << occurrences.size() << endl;

    vector<string> repeats = get_all_repeats(inverted_index, vector<Occurrence>(occurrences.begin(), occurrences.end()));
    cout << "Found " << repeats.size() << " repeated strings";
    if (repeats.size() > 0) {
        cout << " of length " << repeats[0].size();
    }
    cout << endl;
    print_vector("Repeated strings", repeats);
}

// lower_bound/upper_bound example

static void upper_bound_test() {
  int myints[] = {10,20,30,30,20,10,10,20};
  vector<int> v(myints,myints+8);           // 10 20 30 30 20 10 10 20
  vector<int>::iterator low,up;

  sort(v.begin(), v.end());                // 10 10 10 20 20 20 30 30

  low= lower_bound (v.begin(), v.end(), 20); //          ^
  up = upper_bound (v.begin(), v.end(), 20); //                   ^

  cout << "lower_bound at position " << int(low - v.begin()) << endl;
  cout << "upper_bound at position " << int(up - v.begin()) << endl;
 
}

int main() {

    //upper_bound_test();
    test();
    return 0;
}


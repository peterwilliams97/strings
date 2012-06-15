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

    list<string> repeats = get_all_repeats(inverted_index, vector<Occurrence>(occurrences.begin(), occurrences.end()));
    
    cout << "===========================================" << endl;
    cout << "Found " << repeats.size() << " repeated strings";
    if (repeats.size() > 0) {
        cout << " of length " << repeats.front().size();
    }
    cout << endl;
    print_list("Repeated strings", repeats);
}

// lower_bound/upper_bound example

static void upper_bound_test() {
  int myints[] = {10,20,30,30,20,10,10,20};
  vector<int> v(myints, myints+NUMELEMS(myints));           // 10 20 30 30 20 10 10 20
  vector<int>::iterator low,up,over;
                                                //  0  1  2  3  4  5  6  7
  sort(v.begin(), v.end());                     // 10 10 10 20 20 20 30 30

  low= lower_bound(v.begin(), v.end(), 20);    //          ^
  up = upper_bound(v.begin(), v.end(), 20);    //                   ^

  cout << "lower_bound at position " << int(low - v.begin()) << ",val= " << *low << endl;
  cout << "upper_bound at position " << int( up - v.begin()) << ",val= " << *up << endl;
 
  over = upper_bound(v.begin(), v.end(), 30);
  cout << "over=" << *over << endl;
  over = upper_bound (v.begin(), v.end(), 40);
  cout << "over=" << *over << endl;
}

int main() {

    //upper_bound_test();
    test();
    return 0;
}


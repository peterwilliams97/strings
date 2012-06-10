#include <assert.h>
#include <map>
#include <vector>
#include <list>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "utils.h"
#include "inverted_index.h"

using namespace std;

typedef unsigned char byte;
typedef unsigned int offset_t;
typedef offset_t *p_offset_t;

/*
 * http://en.wikipedia.org/wiki/Inverted_index
 */
struct Postings {
    int total_terms;                        // Total # occurences of term in all documents 
    int total_docs;                         // # documents that term occurs in
    vector<int> doc_indexes;                // Indexes of docs that term occurs in  
    map<int, vector<offset_t>> offsets_map; // offsets[i] = offsets of term in document with index i 
    // Optional
    map<int, vector<offset_t>> ends_map;    // ends[i] = offset of end of term in document with index i 
};

struct InvertedIndex {
    // Inverted index for strings across files
    map<string, Postings> _terms;
  
    // _docs[i] = filename of document with index i
    vector<string> _docs;

    InvertedIndex(map<string, Postings> terms, vector<string> docs) { 
        _terms = terms;
        _docs = docs;
    }

    void show(const string title) const {
        cout << "InvertedIndex ===== " << title << endl;
        print_list("_terms", get_keys(_terms));
        print_vector("_docs", _docs);

    }
};

void show_inverted_index(const string title, const InvertedIndex *inverted_index) {
    inverted_index->show(title);
}

/*
 * Return index of doc if in index, otherwise -1
 */
int get_doc_index(const InvertedIndex *inverted_index, string doc) {
    const vector<string> &docs = inverted_index->_docs;
    for (unsigned int i = 0; i < docs.size(); i++) {
        if (docs[i] == doc) {
            return i;
        }
    }
    return -1;
}

static const int NUM_CHARS = 256;

/*
 * Read file named filename into a map of bytes:all offsets of byte in a document 
 *  and return the map 
 */
map<byte, vector<offset_t>> 
get_doc_offsets_map(const string filename) {
	
    map<byte, vector<offset_t>> index;

    for (int i = 0; i < NUM_CHARS; i++) {
        index[i] = vector<offset_t>();
    }
	
    ifstream f;
    f.open(filename, ios::in | ios::binary);
    if (!f.is_open()) {
	cerr << "could not open " << filename << endl;
	return index;
    }

    const int BUF_SIZE = 64 * 1024;
    p_offset_t *buffers = new p_offset_t[NUM_CHARS];
    p_offset_t *ptrs = new p_offset_t[NUM_CHARS];
    for (int i = 0; i < NUM_CHARS; i++) {
        buffers[i] = new offset_t[BUF_SIZE];
	ptrs[i] = buffers[i];
    }
    byte *filebuf = new byte[BUF_SIZE];

    offset_t offset = 0;
    while (!f.eof()) {
	f.read((char *)filebuf, BUF_SIZE);
	streamsize n = f.gcount(); 
	for (streamsize i = 0; i < n; i++) {
	    byte b = filebuf[i];
	    *(ptrs[b]++) = offset++;
	}

	for (int i = 0; i < NUM_CHARS; i++) { 
	    if (ptrs[i] > buffers[i]) {
		size_t old_size = index[i].size();
		index[i].resize(index[i].size() + ptrs[i] - buffers[i]);
		copy(buffers[i], ptrs[i], index[i].begin());
		ptrs[i] = buffers[i];
	    }
			
	}

    }
    f.close();

    // Get rid of all the empty lists
    map<byte, vector<offset_t>> offsets;
    for (map<byte, vector<offset_t>>::iterator it = index.begin(); it != index.end(); it++) {
        if (it->second.size() > 0) {
            offsets[it->first] = it->second;
        }
    }

    cout << "get_doc_offsets_map(" << filename << ") " << offsets.size() << " {";
    for (map<byte, vector<offset_t>>::iterator it = offsets.begin(); it != offsets.end(); it++) {
        cout << it->first << ":" << it->second.size() << ", ";
    }
    cout << "}" << endl;
    return offsets;
}

InvertedIndex 
*make_inverted_index(const vector<string> filenames) {
 
    map<string, Postings> terms;
    list<string> docs;
   
    for (unsigned int i = 0; i < filenames.size(); i++) {
        map<byte, vector<offset_t>> offsets_map = get_doc_offsets_map(filenames[i]);

        if (offsets_map.size() > 0) {
            docs.push_back(filenames[i]);

            for (map<byte, vector<offset_t>>::iterator it = offsets_map.begin(); it != offsets_map.end(); it++) {
                
                byte b = it->first;
                vector<offset_t> &offsets = it->second;
               
                // This should always be tre
                assert(offsets.size() > 0);

                string s(1, (char)b);
                terms[s].total_terms += offsets.size();
                terms[s].total_docs++;
                int doc_idx = docs.size() - 1;  
                terms[s].doc_indexes.push_back(doc_idx);
                vector<offset_t> &current_offsets = terms[s].offsets_map[doc_idx];
                current_offsets.resize(current_offsets.size() + offsets.size());
                //cout << "s='" << s << "',doc_idx=" << doc_idx << ",offsets=" << offsets.size() << ",current_offsets=" << current_offsets.size() << endl;
                copy(offsets.begin(), offsets.end(), current_offsets.begin());

            }
        }
    }

    return new InvertedIndex(terms, vector<string>(docs.begin(), docs.end()));
}

/*
 * Return list of bytes that occur >= specified number of times in docs
 */
vector<byte>
get_repeated_bytes(const InvertedIndex *index, const vector<Occurrence> occurrences) {
    map<string, Postings> terms = index->_terms;
    
    cout << "get_repeated_bytes: terms=" << terms.size() << endl;
    for (map<string, Postings>::iterator it = terms.begin(); it != terms.end(); it++) {
        cout << it->first << ", ";    
    }
    cout << endl;

    list<byte> result;
    for (map<string, Postings>::iterator it = terms.begin(); it != terms.end(); it++) {
         string s = it->first;

         // We are only interested in single byte strings
         if (s.size() == 1) {
             byte b = s[0];
             Postings &postings = it->second; 
             bool match = true;
             for (unsigned int i = 0; i < occurrences.size(); i++) {
                map<int,vector<offset_t>>::iterator ofs = postings.offsets_map.find(occurrences[i].doc_index); 
                if (!(ofs != postings.offsets_map.end() && postings.offsets_map[occurrences[i].doc_index].size() >= occurrences[i].num)) {
                    match = false;
                    break;
                }
             }
             if (match) {
                 result.push_back(b);
             }  
         }
    }
    return vector<byte>(result.begin(), result.end());
}

vector<string>
get_repeated_strings(const vector<byte> &repeated_bytes) {
    list<string> repeated_strings;
    for (vector<byte>::const_iterator it = repeated_bytes.begin(); it != repeated_bytes.end(); it++) {
        repeated_strings.push_back(string(1, (char)*it));
    }
    return vector<string>(repeated_strings.begin(), repeated_strings.end());
}

/*
 * Return true if s + b occurs >= number times
 *  where s is a member of strings and b is a member of bytes
 *  m is size of s for s in strings
 * THIS IS THE INNER LOOP
 */
bool occurs(const vector<offset_t> &strings, offset_t m, const vector<offset_t> &bytes, int num) {
    int num_matches = 0;
    vector<offset_t>::const_iterator bytes_lower = bytes.begin();
    for (vector<offset_t>::const_iterator is = strings.begin(); is != strings.end(); is++) {
        vector<offset_t>::const_iterator next = upper_bound(bytes_lower, bytes.end(), (*is) + m); 
        if ((*next) == (*is) + m + 1) {
            num_matches++;
            if (num_matches >= num) {
                return true;
            }
            next++;
            bytes_lower = next; 
        }
    }
    return false;
}

/*
 * Return true if s + b exists sufficient numbers of times in each document
 *  s and b are guaranteed to be in all occurrneces
 */
bool 
has_sufficient_occurrences(map<string, Postings> &terms, const vector<Occurrence> &occurrences, 
     string s, byte b) {
    unsigned int m = s.size();

    Postings &s_postings = terms[s];
    Postings &b_postings = terms[string(1, (char)b)];

    cout << "has_sufficient_occurrences(s='" << s << "',b=" << b << ")" << endl;
    
    for (unsigned int i = 0; i < occurrences.size(); i++) {
        vector<offset_t> &strings = s_postings.offsets_map[occurrences[i].doc_index];
        vector<offset_t> &bytes = b_postings.offsets_map[occurrences[i].doc_index];

        cout << " strings='" << strings.size() << ",bytes='" << bytes.size() << endl; 

        if (!occurs(strings, m, bytes, occurrences[i].num)) {
            return false;
        }
    }
    return true;
}

/*
 * Return list of strings that are repeated sufficient numbers of time
 * Start with exact match to test the c++ written so fat
 */
vector<string>
get_all_repeats(InvertedIndex *inverted_index, const vector<Occurrence> occurrences) {
    vector<byte> repeated_bytes = get_repeated_bytes(inverted_index, occurrences); 
    vector<string> repeated_strings = get_repeated_strings(repeated_bytes);  

    cout << "repeated_bytes=" << repeated_bytes.size() << ",repeated_strings=" << repeated_strings.size() << endl;

    while (true) {
        list<string> repeated_strings_n1;
        for (vector<string>::iterator is = repeated_strings.begin(); is != repeated_strings.end(); is++) {
            for (vector<byte>::iterator ib = repeated_bytes.begin(); ib != repeated_bytes.end(); ib++) {
                string s = *is;
                byte b = *ib;
                if (has_sufficient_occurrences(inverted_index->_terms, occurrences, s, b)) {
                    repeated_strings_n1.push_back(*is + string(1, (char)*ib));
                }
            } 
        }
        if (repeated_strings_n1.size() == 0) {
            break;
        }
        repeated_strings = vector<string>(repeated_strings_n1.begin(), repeated_strings_n1.end());
    }
    return repeated_strings;
}

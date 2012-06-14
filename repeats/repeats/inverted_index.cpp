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
	
    map<byte, list<offset_t>> offsets_map;

    for (int i = 0; i < NUM_CHARS; i++) {
        offsets_map[i] = list<offset_t>();
    }
	
    ifstream f;
    f.open(filename, ios::in | ios::binary);
    if (!f.is_open()) {
	cerr << "could not open " << filename << endl;
	return map<byte, vector<offset_t>>();
    }

    const int BUF_SIZE = 64 * 1024;
    byte *filebuf = new byte[BUF_SIZE];

    offset_t offset = 0;
    while (!f.eof()) {
	f.read((char *)filebuf, BUF_SIZE);
	streamsize n = f.gcount(); 
	for (streamsize i = 0; i < n; i++) {
	    byte b = filebuf[i];
            offsets_map[b].push_back(offset);
            offset++;
	}
    }

    f.close();
    delete[] filebuf;

    // Get rid of all the empty lists
    map<byte, vector<offset_t>> offsets;
    for (map<byte, list<offset_t>>::iterator it = offsets_map.begin(); it != offsets_map.end(); it++) {
        if (it->second.size() > 0) {
            offsets[it->first] = vector<offset_t>(it->second.begin(), it->second.end());
        }
    }

    cout << "get_doc_offsets_map(" << filename << ") " << offsets.size() << " {";
    for (map<byte, vector<offset_t>>::iterator it = offsets.begin(); it != offsets.end(); it++) {
        cout << it->first << ":" << it->second.size() << ", ";
    }
    cout << "}" << endl;
    
    for (map<byte, vector<offset_t>>::iterator it = offsets.begin(); it != offsets.end(); it++) {
        vector<offset_t> ofs = it->second; 
        if (ofs[0] > 1000) {
            cout << " *** " << it->first << ":" << ofs[0] << endl;
        }
    }
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
                assert(current_offsets[0] < 1000);

            }
        }
        cout << " Added " << filenames[i] << " to terms" << endl;
    }

    return new InvertedIndex(terms, vector<string>(docs.begin(), docs.end()));
}

/*
 * Return maps of bytes and their offset in docs that occur >= specified number of times in docs
 */
map<string, Postings>
get_repeated_bytes(const InvertedIndex *index, const vector<Occurrence> occurrences) {
    map<string, Postings> terms = index->_terms;
    
    cout << "get_repeated_bytes: terms=" << terms.size() << endl;
    for (map<string, Postings>::iterator it = terms.begin(); it != terms.end(); it++) {
        cout << it->first << ", ";    
    }
    cout << endl;

    map<string, Postings> result;
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
                 result[s] = it->second;
             }  
         }
    }
    return result;
}

/*
 * Return true if s + b occurs >= num times
 *  where s is a member of strings and b is a member of bytes
 *  m is size of s for s in strings
 * THIS IS THE INNER LOOP
 *
 * Basic idea is to keep 2 pointer and move the one behind and record matches of 
 *  *is + m == *ib
 */
bool 
occurs(const vector<offset_t> &strings, offset_t m, const vector<offset_t> &bytes, int num) {
    
    int num_matches = 0;
    vector<offset_t>::const_iterator ib = bytes.begin();
    vector<offset_t>::const_iterator is = strings.begin();

#if DEBUG        
    cout << " bytes.back()=" << bytes.back() << " strings.back()=" << strings.back() << endl;
    print_vector( "  strings", strings);
    print_vector( "    bytes", bytes);
#endif

    while (ib < bytes.end() && is < strings.end()) {
       
        // Largest value in bytes <= *is + m
        ib = get_lteq(ib, bytes.end(), *is + m);
        if (ib == bytes.end()) {
            break;
        }

        if (*ib == *is + m) {
            num_matches++;
#if DEBUG
            cout << " match " << num_matches << " at is = " << *is << " ib = " << *ib << endl;
#endif
            // optimization
            //if (num_matches >= num) {
            //    break;
            //}
            is++;
            continue;
        } 
        
        if (is + 1 >= strings.end()) {
            break;
        }
        // *ib < *is + m. move is ahead.
        is = get_gt(is+1, strings.end(), *ib - m);
       
    }

    return num_matches >= num;
}

/*
 * Return true if s + b exists sufficient numbers of times in each document
 *  s and b are guaranteed to be in all occurrences
 */
bool 
has_sufficient_occurrences(const vector<Occurrence> &occurrences,
                        map<string, Postings> &strings_map, const string s,
                        map<string, Postings> &bytes_map, const string b,
                        bool debug) {
    unsigned int m = s.size();

    Postings &s_postings = strings_map[s];
    Postings &b_postings = bytes_map[b];

    //cout << "has_sufficient_occurrences(s='" << s << "',b=" << b << ")" << endl;
    
    for (unsigned int i = 0; i < occurrences.size(); i++) {
        vector<offset_t> &strings = s_postings.offsets_map[occurrences[i].doc_index];
        vector<offset_t> &bytes = b_postings.offsets_map[occurrences[i].doc_index];

       // cout << " strings='" << strings.size() << ",bytes='" << bytes.size() << endl; 
        if (debug) {
            cout << " ----- " << endl;
            cout << s << "  ";
            print_vector("strings", strings);
            cout << b << "  ";
            print_vector("bytes", bytes);
            cout << " ----- " << endl;
        }
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
list<string>
get_all_repeats(InvertedIndex *inverted_index, const vector<Occurrence> occurrences) {
    map<string, Postings> repeated_bytes_map = get_repeated_bytes(inverted_index, occurrences); 
    map<string, Postings> repeated_strings_map = copy_map(repeated_bytes_map);  
        
    cout << "get_all_repeats: repeated_bytes=" << repeated_bytes_map.size() << ",repeated_strings=" << repeated_strings_map.size() << endl;

    list<string> repeated_bytes = get_keys(repeated_bytes_map);

    for (offset_t n = 1; ; n++) {
        list<string> repeated_strings = get_keys(repeated_strings_map);
        cout << "get_all_repeats: num repeated strings=" << repeated_strings.size() << ", len= " << n ;
        print_list("  strings", repeated_strings);
        
        map<string, Postings>  repeated_strings_n1;
        
        for (list<string>::iterator is = repeated_strings.begin(); is != repeated_strings.end(); is++) {
            for (list<string>::iterator ib = repeated_bytes.begin(); ib != repeated_bytes.end(); ib++) {
                string s = *is;
                string b = *ib;
                bool debug = (s == "re" && b == "p") && false;
                if (debug) {
                    cout << " $$$ re case" << endl;
                }
                if (has_sufficient_occurrences(occurrences, repeated_strings_map, s, repeated_bytes_map, b, debug)) {
                    repeated_strings_n1[s + b] = repeated_strings_map[s];
                } else {
                    //cout << s+b << " did not occur sufficient times";
                }
            } 
        }
        if (repeated_strings_n1.size() == 0) {
            break;
        }
        repeated_strings_map = repeated_strings_n1;
    }
    return get_keys(repeated_strings_map);
}

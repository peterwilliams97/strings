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
    int _total_terms;                        // Total # occurences of term in all documents 
    vector<int> _doc_indexes;                // Indexes of docs that term occurs in  
    map<int, vector<offset_t>> _offsets_map; // offsets[i] = offsets of term in document with index i 
    // Optional
    map<int, vector<offset_t>> _ends_map;    // ends[i] = offset of end of term in document with index i 

    Postings() : _total_terms(0) {}

    void add_offsets(int doc_index, const vector<offset_t> &offsets) {
        _doc_indexes.push_back(doc_index);
        _offsets_map[doc_index] = offsets;
        _total_terms += offsets.size();
    }

    unsigned int size() const { return _offsets_map.size(); }

    bool empty() const { return size() == 0; }
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
	
    map<byte, list<offset_t>> _offsets_map;

    for (int i = 0; i < NUM_CHARS; i++) {
        _offsets_map[i] = list<offset_t>();
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
            _offsets_map[b].push_back(offset);
            offset++;
	}
    }

    f.close();
    delete[] filebuf;

    // Get rid of all the empty lists
    map<byte, vector<offset_t>> offsets;
    for (map<byte, list<offset_t>>::iterator it = _offsets_map.begin(); it != _offsets_map.end(); it++) {
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
        map<byte, vector<offset_t>> _offsets_map = get_doc_offsets_map(filenames[i]);

        if (_offsets_map.size() > 0) {
            docs.push_back(filenames[i]);

            for (map<byte, vector<offset_t>>::iterator it = _offsets_map.begin(); it != _offsets_map.end(); it++) {
                
                byte b = it->first;
                vector<offset_t> &offsets = it->second;
               
                // This should always be tre
                assert(offsets.size() > 0);

                string s(1, (char)b);
                terms[s]._total_terms += offsets.size();
                int doc_idx = docs.size() - 1;  
                terms[s]._doc_indexes.push_back(doc_idx);
                vector<offset_t> &current_offsets = terms[s]._offsets_map[doc_idx];
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
                map<int,vector<offset_t>>::iterator ofs = postings._offsets_map.find(occurrences[i].doc_index); 
                if (!(ofs != postings._offsets_map.end() && postings._offsets_map[occurrences[i].doc_index].size() >= occurrences[i].num)) {
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
 * Return ordered vector of offsets of strings s+b in document where 
 *      strings is ordered vector of offsets of strings s in document
 *      bytes is ordered vector of offsets of strings b in document
 *      m is length of s
 *
 * THIS IS THE INNER LOOP
 *
 * Basic idea is to keep 2 pointer and move the one behind and record matches of 
 *  *is + m == *ib
 */
const vector<offset_t>  
get_doc_offsets(const vector<offset_t> &strings, offset_t m, const vector<offset_t> &bytes) {
    
    vector<offset_t>::const_iterator ib = bytes.begin();
    vector<offset_t>::const_iterator is = strings.begin();
    list<offset_t> sb;

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
            sb.push_back(*is);
#if DEBUG
            cout << " match " << num_matches << " at is = " << *is << " ib = " << *ib << endl;
#endif
            is++;
            continue;
        } 
        
        if (is + 1 >= strings.end()) {
            break;
        }
        // *ib < *is + m. move is ahead.
        is = get_gt(is+1, strings.end(), *ib - m);
    }

    return vector<offset_t>(sb.begin(), sb.end());
}

/*
 * Return Posting for s + b is s+b exists sufficient numbers of times in each document
 *  otherwise an empty Postings
 *  s and b are guaranteed to be in all occurrences
 */
Postings 
get_postings(const vector<Occurrence> &occurrences,
              map<string, Postings> &strings_map, const string s,
              map<string, Postings> &bytes_map, const string b) {
    
    unsigned int m = s.size();
    Postings &s_postings = strings_map[s];
    Postings &b_postings = bytes_map[b];
    Postings sb_postings;

    for (unsigned int i = 0; i < occurrences.size(); i++) {
        int doc_index = occurrences[i].doc_index; 
        vector<offset_t> &strings = s_postings._offsets_map[doc_index];
        vector<offset_t> &bytes = b_postings._offsets_map[doc_index];

        vector<offset_t> sb_offsets = get_doc_offsets(strings, m, bytes);
        if (sb_offsets.size() < occurrences[i].num) {
            // Empty map signals no match
            return Postings();
        }

        // cout << "   matched " << s + b << " for doc " << doc_index << endl;
        sb_postings.add_offsets(doc_index, sb_offsets);
    }

    // cout << " matched " << s + b + " for " << sb_postings.size() << " docs" << endl;
    return sb_postings;
}

/*
 * Return list of strings that are repeated sufficient numbers of time
 * Start with exact match to test the c++ written so far
 * 
 * Basic idea
 *  repeated_strings_map contains repeated strings (worst case 4 x size of all docs) 
 *  in each inner loop over repeated_bytes 
 *      repeated_bytes_map[s] is replaced by < 256 repeated_bytes_map[s+b]
 *      total size cannot grow because all s+b strings are contained in repeated_bytes_map[s]
 *      (NOTE Could be smarter and use repeated_bytes_map for strings of length 1)
 *      strings that do not occur often enough in all docs are filtered out
 *
 */
list<string>
get_all_repeats(InvertedIndex *inverted_index, const vector<Occurrence> occurrences) {
    map<string, Postings> repeated_bytes_map = get_repeated_bytes(inverted_index, occurrences); 
    map<string, Postings> repeated_strings_map = copy_map(repeated_bytes_map);  
        
    cout << "get_all_repeats: repeated_bytes=" << repeated_bytes_map.size() << ",repeated_strings=" << repeated_strings_map.size() << endl;

    list<string> repeated_bytes = get_keys(repeated_bytes_map);
    list<string> repeated_strings = get_keys(repeated_strings_map);

    for (offset_t n = 1; ; n++) {
       
        cout << "get_all_repeats: num repeated strings=" << repeated_strings.size() << ", len= " << n ;
        print_list("  strings", repeated_strings);
        
        map<string, Postings>  repeated_strings_n1;
        
        for (list<string>::iterator is = repeated_strings.begin(); is != repeated_strings.end(); is++) {
            string s = *is;

            // Replace repeated_strings_map[s] with repeated_strings_map[s+b] for all b
            // This cannot increase total number of offsets as each s+b starts with s
            for (list<string>::iterator ib = repeated_bytes.begin(); ib != repeated_bytes.end(); ib++) {
                string b = *ib;
                
                Postings postings = get_postings(occurrences, repeated_strings_map, s, repeated_bytes_map, b);
                if (!postings.empty()) { 
                    repeated_strings_map[s + b] = postings;
                } 
            }
            repeated_strings_map.erase(s);

           // print_list(" == intermediate strings",  get_keys(repeated_strings_map));
        }


        // If not matches then we are doene
        if (repeated_strings_map.size() == 0) {
            break;
        }
        
        repeated_strings = get_keys(repeated_strings_map);
    }

    return repeated_strings;
}

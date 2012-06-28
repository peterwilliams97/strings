/*
 * INNER_LOOPs
 *  1 is approx twice fast as 1
 *  3 seems a little faster than 2
 *
 *  5 x 2 MB
 *  --------
 *   1pf   135 sec
 *   2pf   160
 *   3pf   160
 *
 *  5 x 4 MB
 *  --------
 *   1pf  257 sec
 *   2pf  315 
 *   3pf  335
 *
 *  5 x 8 MB
 *  --------
 *   1pf  505 sec
 *   2pf  675 
 *   3pf  660
*
 *  5 x 20 MB
 *  --------
 *   1pf 1195 sec
 *   2pf 1784   
 *   3pf 1618  
 * 
 *  10 x 2 MB
 *  --------
 *   1    150 sec 
 *   2     36
 *   3     35   
 *
 *  5 x 20 MB
 *  --------
 *   1    1295   
 *   2    320  860 sec 
 *   3    296
 *   3pf  231  800   (pre-filter == check for match of last n chars)
 */
#define INNER_LOOP 4

#include <assert.h>
#include <regex>
#include <iostream>

#include "mytypes.h"
#include "utils.h"
#include "timer.h"
#include "inverted_index.h"

using namespace std;

// Specify number of times a term must occur in a doc
struct Occurrence {
    std::string _doc_name;      // Document name
    unsigned int _num;          // Number of occurrences
    size_t _size;               // Size of document in bytes     
    Occurrence(const std::string doc_name, unsigned int num, size_t size) :
        _doc_name(doc_name), _num(num), _size(size) {}  
    Occurrence() : _doc_name(""), _num(0), _size(0) {} 
    // Size of each repeat
    double repeat_size() { return (double)_size/(double)_num; }
};

// How min num repeats are encoded in document names 
static const string PATTERN_REPEATS = "repeats=(\\d+)";

/* 
 * Comparison function to sort documents in order of size of repeat
 */
static bool 
comp_occurrence(Occurrence occ1, Occurrence occ2) { 
     return occ1.repeat_size() < occ2.repeat_size(); 
}

/*
 * Given a vector of filenames PATTERN_REPEATS name encoding, return the
 *  corresponding vector of Occurrences
 * Vector is sorted in order of increasing repeat size as smaller repeat
 *  sizes are more selective
 */
static vector<Occurrence> 
get_occurrences(const vector<string> filenames) {

#if VERBOSITY >= 1
    cout << "get_occurrences: " << filenames.size() << " files" << endl;
#endif  
    vector<Occurrence> occurrences;
    regex re_repeats(PATTERN_REPEATS);

    for (vector<string>::const_iterator it = filenames.begin(); it < filenames.end(); it++) {
        const string &fn = *it; 
        cmatch matches;
        if (regex_search(fn.c_str(), matches, re_repeats)) {
            occurrences.push_back(Occurrence(fn, string_to_int(matches[1]), get_file_size(fn)));
        } else {
            cerr << "file " << fn << " does not match pattern " << PATTERN_REPEATS << endl;
        }
    } 
       
    sort(occurrences.begin(), occurrences.end(), comp_occurrence);
#if VERBOSITY >= 1
    for (vector<Occurrence>::iterator it = occurrences.begin(); it < occurrences.end(); it++) {
        cout << it - occurrences.begin() << ": " << it->_doc_name << ", " << it->_num << ", " << it->_size << endl;
    }
#endif
    return occurrences;
}

/*
 * A Postings is a list of lists of offsets of a particular term
 *  in all documents in our corpus.
 *
 * NOTE: A term is represented as a string and can represent a string 
 *  or a byte
 *
 *  _offsets_map[i] stores the offsets in document i
 *  
 * http://en.wikipedia.org/wiki/Inverted_index
 */
struct Postings {
    // Total # occurences of term in all documents
    int _total_terms;  

    // Indexes of docs that term occurs in
    vector<int> _doc_indexes;      

    // _offsets_map[i] = offsets of term in document with index i 
    //  Each offsets[i] is sorted smallest to largest
    map<int, vector<offset_t>> _offsets_map;  

    // Optional
    // ends[i] = offset of end of term in document with index i 
    //map<int, vector<offset_t>> _ends_map;   

    // All fields are zero'd on construction
    Postings() : _total_terms(0) {}

    // Add offsets for doc with index doc_offset to Postings
    void add_offsets(int doc_index, const vector<offset_t> &offsets) {
        _doc_indexes.push_back(doc_index);
        _offsets_map[doc_index] = offsets;
        _total_terms += offsets.size();
    }

    // Return number of documents whose offsets are stored in Posting
    unsigned int size() const { return _offsets_map.size(); }

    // Return true if no documents are encoding in Posting
    bool empty() const { return size() == 0; }
};

/*
 * An InvertedIndex is a map of Postings of a set of terms accross 
 *  all documents in a corpus.
 *
 *  _postings_map[term] stores all the offsets of term in all the 
 *      docuements in the corpus 
 *  
 * Typical usage is to construct an initial InvertedIndex whose terms
 *  are all bytes that occur in the corpus then to replace these 
 *  with each string that occurs in the corpus. This is done
 *  bottom-up, replacing _postings_map[s] with _postings_map[s+b]
 *  for all bytes b to get from terms of length n to terms of 
 *  length n+1
 */
struct InvertedIndex {
    // _postings_map[term] is the Postings of string term 
    map<string, Postings> _postings_map;
  
    // _docs_map[i] = filename + min occurrences with of document index i. 
    //  The Postings in _postings_map index into this map
    map<int, Occurrence> _docs_map;

    set<string> _allowed_terms;

    InvertedIndex() {
        // Make all bytes allowed
        for (int b = 0; b < 256; b++) {
            _allowed_terms.insert(string(1,(byte)b));
        }
    }
    
    /* 
     * Add term offsets from a document to the inverted index
     *  Trim _postings_map keys that are not in term_offsets
     */
    void add_doc(const Occurrence &occurrence, map<string, vector<offset_t>> &term_offsets) {
        // Remove keys in _postings_map that are not keys of term_offsets
        set<string> common_keys = get_intersection(_allowed_terms, get_keys_set(term_offsets));
        trim_keys(_postings_map, common_keys);

        int doc_index = _docs_map.size();
        _docs_map[doc_index] = occurrence;
             
        for (set<string>::iterator it = common_keys.begin(); it != common_keys.end(); it++) {
            const string &term = *it;
            vector<offset_t> &offsets = term_offsets[term];
            _postings_map[term].add_offsets(doc_index, offsets);
        }
    }

    void show(const string title) const {
#if VERBOSITY >= 2
        cout << " InvertedIndex ===== " << title << endl;
        print_list(" _postings_map", get_keys(_postings_map));
        print_list(" _docs_map", get_keys(_docs_map));
        print_set(" _allowed_terms", _allowed_terms);
#endif
   }
};

/*
 * Write contents of inverted_index to stdout
 */
void 
show_inverted_index(const string title, const InvertedIndex *inverted_index) {
    inverted_index->show(title);
}

/*
 * Return index of document named doc_name if it is in inverted_index, 
 *  otherwise -1
 * !@#$ Clean this up with a reverse map
 */
static int 
get_doc_index(InvertedIndex *inverted_index, const string doc_name) {
    map<int, Occurrence> &docs_map = inverted_index->_docs_map;
    for (int i = 0; i < (int)docs_map.size(); i++) {
        string &doc = docs_map[(const int)i]._doc_name;
        if (doc == doc_name) {
            return i;
        }
    }
    return -1;
}

static bool 
check_sorted(const vector<offset_t> &offsets) {
    for (unsigned int i = 1; i < offsets.size(); i++) {
        assert(offsets[i] > offsets[i-1]);
        if (offsets[i] <= offsets[i-1]) {
            return false;
        }
    }    
    return true;
}

static const int NUM_CHARS = 256;

/*
 * Read file named filename into a map of bytes:(all offsets of byte in a document)
 *  and return the map 
 * Only read in offsets of bytes in allowed_bytes that  occur >= min_occurrences 
 *  times
 */
static map<string, vector<offset_t>> 
get_doc_offsets_map(const string filename, set<string> &allowed_terms, unsigned int min_occurrences) {
	
    offset_t counts[256];
    memset(&counts, 0, sizeof(counts));
	
    size_t length = get_file_size(filename);
    byte *data = read_file(filename);
    byte *end = data + length;
    
    // Pass through the document once to get counts of all bytes
    for (byte *p = data; p < end; p++) {
        counts[*p]++;
    }

    // valid_bytes are those with sufficient counts
    set<string> valid_bytes;
    for (int b = 0; b < 256; b++) {
        if (counts[b] >= min_occurrences) {
            valid_bytes.insert(string(1,b));
        }
    } 

    // We use only the bytes that are valid for all documents so far/
    allowed_terms = get_intersection(allowed_terms, valid_bytes);

    // We have counts so we can pre-allocate data structures
    map<string, vector<offset_t>> offsets_map;
    bool allowed_bytes[256];
    vector<offset_t>::iterator offsets_ptr[256];
    for (int b = 0; b < 256; b++) {
        string s(1,b);
        bool allowed = allowed_terms.find(s) != allowed_terms.end();
        allowed_bytes[b] = allowed;
        if (allowed) {
            offsets_map[s] = vector<offset_t>(counts[b]);
            offsets_ptr[b] = offsets_map[s].begin();
        }
    }
    
    // Pass through the document a second time and read in the bytes
    offset_t ofs = 0;
    for (byte *p = data; p < end; p++) {
        if (allowed_bytes[*p]) {
            *(offsets_ptr[*p]) = ofs;
            ofs++;
            offsets_ptr[*p]++;
        }
    }

    delete[] data;
    
    // Report what was read to stdout
#if VERBOSITY >= 2
    cout << "get_doc_offsets_map(" << filename << ") " << offsets_map.size() << " {";
    for (map<string, vector<offset_t>>::iterator it = offsets_map.begin(); it != offsets_map.end(); it++) {
        cout << it->first << ":" << it->second.size() << ", ";
        //check_sorted(it->second);
    }
    cout << "}" << endl;
#endif    
    return offsets_map;
}

/*
 * Create the InvertedIndex corresponding to filenames 
 */
InvertedIndex 
*create_inverted_index(const vector<string> &filenames) {
 
    vector<Occurrence> occurrences = get_occurrences(filenames);

    map<string, Postings> terms;
    list<string> docs;

    InvertedIndex *inverted_index = new InvertedIndex();
   
    for (vector<Occurrence>::const_iterator it = occurrences.begin(); it != occurrences.end(); it++) {
        const Occurrence &occ = *it;
        map<string, vector<offset_t>> offsets_map = get_doc_offsets_map(occ._doc_name, inverted_index->_allowed_terms, occ._num);
        if (offsets_map.size() > 0) {
            inverted_index->add_doc(occ, offsets_map);
        }     
               
#if VERBOSITY >= 1
        cout << " Added " << occ._doc_name << " to inverted index" << endl;
#endif
#if VERBOSITY >= 2
        inverted_index->show(occ._doc_name);
#endif
    }

    return inverted_index;
}

void
delete_inverted_index(InvertedIndex *inverted_index) {
    delete inverted_index;
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
inline  const vector<offset_t>  
get_sb_offsets(const vector<offset_t> &strings, offset_t m, const vector<offset_t> &bytes) {
    vector<offset_t> sb;
    vector<offset_t>::const_iterator ib = bytes.begin();
    vector<offset_t>::const_iterator is = strings.begin();
 
#if INNER_LOOP == 1    
    vector<offset_t>::const_iterator b_end = bytes.end(); 
    vector<offset_t>::const_iterator s_end = strings.end(); 
           
    while (ib < b_end && is < s_end) {
        offset_t is_m = *is + m;
        if (*ib == is_m) {
            sb.push_back(*is);
            is++;
        } else if (*ib < is_m) {
            while (ib < b_end && *ib < is_m) {
                ib++;
            }
        } else {
            offset_t ib_m =  *ib - m;
            while (is < s_end && *is < ib_m) {
                is++;
            }
        }
    }

#elif INNER_LOOP == 2
 
    while (ib < bytes.end() && is < strings.end()) {
        if (*ib == *is + m) {
            sb.push_back(*is);
            is++;
        } else if (*ib < *is + m) {
            ib = get_gteq(ib, bytes.end(), *is + m);
        } else {
            is = get_gteq(is, strings.end(), *ib - m);
        }
    }

#elif INNER_LOOP == 3
    
    // Performance about same for 256, 512 when num chars is low
    // !@#$ Need to optimize this
    // The next power 2 calculation slows 2 MB test 35 sec => 42 sec!
    //size_t step_size_b = next_power2((double)(bytes.back() - bytes.front())/(double)bytes.size());
    //size_t step_size_s = next_power2((double)(strings.back() - strings.front())/(double)strings.size());

    size_t step_size_b = 512;
    size_t step_size_s = 512; 

    while (ib < bytes.end() && is < strings.end()) {
        
        if (*ib == *is + m) {
            sb.push_back(*is);
            is++;
        } else if (*ib < *is + m) {
            ib = get_gteq2(ib, bytes.end(), *is + m, step_size_b);
        } else {
            is = get_gteq2(is, strings.end(), *ib - m, step_size_s);
        }
    }

#elif INNER_LOOP == 4    
    vector<offset_t>::const_iterator b_end = bytes.end(); 
    vector<offset_t>::const_iterator s_end = strings.end(); 
     
    if (m == 1) {
        while (ib < b_end && is < s_end) {
            offset_t is_m = *is + m;
            if (*ib == is_m) {
                sb.push_back(*is);
                is++;
            } else if (*ib < is_m) {
                while (ib < b_end && *ib < is_m) {
                    ib++;
                }
            } else {
                offset_t ib_m =  *ib - m;
                while (is < s_end && *is < ib_m) {
                    is++;
                }
            }
        }
    } else {
        size_t step_size_b = 512; 
        while (ib < b_end && is < s_end) {
            offset_t is_m = *is + m;
            if (*ib == is_m) {
                sb.push_back(*is);
                is++;
            } else if (*ib < is_m) {
                 ib = get_gteq2(ib, b_end, *is + m, step_size_b);
            } else {
                offset_t ib_m =  *ib - m;
                while (is < s_end && *is < ib_m) {
                    is++;
                }
            }
        }
    }


#endif
    return sb;
}
/*
 * Return Posting for s + b if s+b exists sufficient numbers of times in each document
 *  otherwise an empty Postings
 *  s and b are guaranteed to be in all occurrences
 */
Postings 
get_sb_postings(InvertedIndex *inverted_index,
             map<string, Postings> &strings_map, const string s,
             const string b) {
                 
    unsigned int m = s.size();
    Postings &s_postings = strings_map[s];
    Postings &b_postings = inverted_index->_postings_map[b];
    Postings sb_postings;

    map<int, Occurrence> &docs_map = inverted_index->_docs_map;
    for (map<int, Occurrence>::iterator it = docs_map.begin(); it != docs_map.end(); it++) {
        int doc_index = it->first; 
        vector<offset_t> &strings = s_postings._offsets_map[doc_index];
        vector<offset_t> &bytes = b_postings._offsets_map[doc_index];

        vector<offset_t> sb_offsets = get_sb_offsets(strings, m, bytes);
        if (sb_offsets.size() < it->second._num) {
            // Empty map signals no match
            return Postings();
        }

        sb_postings.add_offsets(doc_index, sb_offsets);
    }

#if VERBOSITY >= 2
    cout << " matched '" << s + b + "' for " << sb_postings.size() << " docs" << endl;
#endif
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
vector<string>
get_all_repeats(InvertedIndex *inverted_index) {

    // Postings map of strings of length 1 
    map<string, Postings> &repeated_bytes_map = inverted_index->_postings_map; 

    // Postings map of strings of length n+1 from strings of length n 
    map<string, Postings> repeated_strings_map = copy_map(repeated_bytes_map);  

#if VERBOSITY >= 1
    cout << "get_all_repeats: repeated_bytes=" << repeated_bytes_map.size() << ",repeated_strings=" << repeated_strings_map.size() << endl;
#endif
    vector<string> repeated_bytes = get_keys_vector(repeated_bytes_map);
    vector<string> repeated_strings = get_keys_vector(repeated_strings_map);

    // Each pass through this for loop builds strings of length n+2 from 
    for (offset_t n = 1; ; n++) {
       
#if VERBOSITY >= 1 
        // Report progress to stdout
        cout << "--------------------------------------------------------------------------" << endl;
        cout << "get_all_repeats: num repeated strings=" << repeated_strings.size() 
             << ", len= " << n 
             << ", time= " << get_elapsed_time() << endl;
#endif
#if VERBOSITY >= 2
        print_vector("repeated_strings", repeated_strings, 40);
#endif   
        // Construct all possible length n+1 strings from existing length n strings in valid_strings
        // and filter out length n+1 strings that don't end with an existing length n string
        // valid_strings[s][b] is later converted to s+b: s is length n, b is length 1
        map<string,vector<string>> valid_strings;
        for (vector<string>::iterator is = repeated_strings.begin(); is != repeated_strings.end(); is++) {
            string &s = *is;
            vector<string> offsets;
            for (vector<string>::iterator ib = repeated_bytes.begin(); ib != repeated_bytes.end(); ib++) {
                string &b = *ib;
                if (binary_search(repeated_strings.begin(), repeated_strings.end(), (s+b).substr(1))) {
                    offsets.push_back(b);
                 }             
            }
            if (offsets.size() > 0) {
                valid_strings[s] = offsets;
            }
        }
               
#if VERBOSITY >= 1 
        //print_vector("   valid_strings", get_keys_vector(valid_strings));
        cout << repeated_strings.size() << " strings * "
             << repeated_bytes.size() << " bytes = "
             << repeated_strings.size() * repeated_bytes.size() << " vs " 
             << get_map_vector_size(valid_strings) << " valid_strings" << endl;
        
#endif
        // Remove from repeated_strings_map the offsets all the length n strings that have won't be 
        // used to construct length n+1 strings below
        for (vector<string>::iterator is = repeated_strings.begin(); is != repeated_strings.end(); is++) {
            if (valid_strings.find(*is) == valid_strings.end()) {
                repeated_strings_map.erase(*is);
            }
        }

        // Replace repeated_strings_map[s] with repeated_strings_map[s+b] for all b in bytes that
        // have survived the valid_strings filtering above
        // This cannot increase total number of offsets as each s+b starts with s
        for (map<string,vector<string>>::iterator iv = valid_strings.begin(); iv != valid_strings.end(); iv++) {
            string s = iv->first;
            vector<string> bytes = iv->second;
            for (vector<string>::iterator ib = bytes.begin(); ib != bytes.end(); ib++) {
                string &b = *ib;
                Postings postings = get_sb_postings(inverted_index, repeated_strings_map, s, b);
                if (!postings.empty()) { 
                    repeated_strings_map[s + b] = postings;
                } 
            }
            repeated_strings_map.erase(s);
        }
        
        // If there are no matches then we were done in the last pass
        if (repeated_strings_map.size() == 0) {
            break;
        }
        
        repeated_strings = get_keys_vector(repeated_strings_map);
    }

    return repeated_strings;
}

static struct VersionInfo {
    VersionInfo() {
        cout << "INNER_LOOP=" << INNER_LOOP << endl;
    };
} _version_info;

#ifdef NOT_DEFINED
#endif // #ifdef NOT_DEFINED
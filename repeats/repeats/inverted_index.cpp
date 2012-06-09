#include <map>
#include <vector>
#include <list>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

typedef unsigned char byte;
typedef unsigned int offset_t;
typedef offset_t *p_offset_t;

/*
 * http://en.wikipedia.org/wiki/Inverted_index
 */
struct Postings {
    int  total_terms;                   // Total # occurences of term in all documents 
    int total_docs;                     // # documents that term occurs in
    vector<int> doc_indexes;           // Indexes of docs that term occurs in  
    map<int,vector<offset_t>> offsets_map;  // offsets[i] = offsets of term in document with index i      
};

struct InvertedIndex {
    // Inverted index for bytes across files
    map<byte, Postings> _terms;
  
    // doc_list[i] = filename of document with index i
    vector<string> _docs;

    InvertedIndex(map<byte, Postings> terms, vector<string> docs) { 
        _terms = terms;
        _docs = docs;
    }
};

static const int NUM_CHARS = 256;

/*
 * Read file named filename into a map of bytes:all offsets of byte in a document 
 *  and return the map 
 */
map<byte, std::vector<offset_t>> 
make_doc_inverted_index(const string filename) {
	
    map<byte, std::vector<offset_t>> index;

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
		std::copy(buffers[i], ptrs[i], index[i].begin());
		ptrs[i] = buffers[i];
	    }
			
	}

    }
    f.close();
    return index;
}

InvertedIndex 
make_inverted_index(const vector<string> filenames) {
 
    map<byte, Postings> terms;
    list<string> docs;
   
    for (unsigned int i = 0; i < filenames.size(); i++) {
        map<byte, vector<offset_t>> offsets = make_doc_inverted_index(filenames[i]);

        if (offsets.size() > 0) {
            docs.push_back(filenames[i]);

            for (map<byte, vector<offset_t>>::iterator it = offsets.begin(); it != offsets.end(); it++) {

                byte b = it->first;
                vector<offset_t> &offsets = it->second;
                terms[b].total_terms += offsets.size();
                terms[b].total_docs++;
                int doc_idx = docs.size() - 1;  
                terms[b].doc_indexes.push_back(doc_idx);
                copy(offsets.begin(), offsets.end(), terms[b].offsets_map[doc_idx].begin());
            }
        }
    }

    return InvertedIndex(terms, vector<string>(docs.begin(), docs.end()));
}

// Specify number of times a term must occur in a doc
struct Occurrence {
    int doc_index;          // Index into docs
    unsigned int num;       // Number of occurrences
};

/*
 * Return list of bytes that occur >= specified number of times in docs
 */
vector<byte>
get_repeated_bytes(InvertedIndex *index, vector<Occurrence> occurrences) {
    map<byte, Postings> terms = index->_terms;
    list<byte> result;
    for (map<byte, Postings>::iterator it = terms.begin(); it != terms.end(); it++) {
         byte b = it->first;
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
    return vector<byte>(result.begin(), result.end());
}


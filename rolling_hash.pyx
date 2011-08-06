from __future__ import division
"""
Rolling hash implementation

Use Cython: http://docs.cython.org/
http://docs.cython.org/src/quickstart/build.html

To build:
python setup.py build_ext --inplace

"""

#http://wiki.cython.org/DynamicMemoryAllocation
from libc.stdlib cimport malloc, calloc, free

#http://trac.cython.org/cython_trac/attachment/ticket/314/array.pxd
cdef extern from "stdlib.h" nogil:
    void *memset(void *str, int c, size_t n)
    void *memcpy(void *dst, void *src, size_t n)

DEF INT_SIZE = 4
DEF INT_BITS = INT_SIZE * 8

# Size of the rolling hash table. Smallest 5 digit prime
DEF HASH_SIZE = 10009
# Memory storage for hash table. Round up to multiple of 32 
DEF HASH_STORAGE = 10016

# The "radix" for the Rabin-Karp rolling hash
# http://en.wikipedia.org/wiki/Rolling_hash
DEF D = 31


# https://github.com/lemire/rollinghashjava/blob/master/src/rollinghash/RabinKarpHash.java
cdef _mod(unsigned int d, unsigned int m):
    """Return d^m """
    cdef unsigned int d_to_n = 1
    for i in range(m):
        d_to_n *= d
    return d_to_n

cdef _get_simple_hash(unsigned char text[], int text_len, int offset):
    """Return hash of text[offset:text_len]"""
    cdef unsigned int t = 0 
    for i in range(text_len): 
        t = (t * D + text[i+offset]) 
    return t

cdef _get_rolling_hash(unsigned char text[], int text_len, int pattern_len, unsigned int hashes[]):
    """Return hashes of text[offset:offset+pattern_len] for offset in [0,text_len-pattern_len]"""
    
    cdef unsigned int h = _mod(D, pattern_len-1)
    print 'D = %d, pattern_len = %d, h=%d' % (D, pattern_len, h)
    
    cdef unsigned int t = _get_simple_hash(text, pattern_len, 0) 
    hashes[0] = t
    assert(t != 0)
      
    for i in range(text_len - pattern_len):
        t = t - (text[i]*h)
        t = (t * D + text[i+pattern_len]) 
        assert(t == _get_simple_hash(text, pattern_len, i+1))
        if t == 0:
            print 'i= ', i
        assert(t!= 0)
        hashes[i+1] = t

DEF TEST_SIZE = 1000 

def test_rolling_hash(): 
    """Test that rolling hashes are the same as the simple hashes"""
    cdef unsigned char text[TEST_SIZE]
    cdef unsigned int hashes[TEST_SIZE]
    memset(text, 0, sizeof(text))
    memset(hashes, 0, sizeof(hashes))

    pattern = 'abcd '
    pattern_len = len(pattern)

    base_hashes_list = []
    for i in range(pattern_len):
        for j in range(pattern_len):
            text[j] = ord(pattern[(i+j) % pattern_len])
        _get_rolling_hash(text, pattern_len, pattern_len, hashes)
        base_hashes_list.append(hashes[0])

    for i in range(TEST_SIZE):
        text[i] = ord(pattern[i % pattern_len])
    _get_rolling_hash(text, TEST_SIZE, pattern_len, hashes)

    num_hashes = TEST_SIZE - pattern_len + 1
    hashes_list = []
    for i in range(TEST_SIZE - pattern_len + 1):
        hashes_list.append(hashes[i])

    print 'TEST_SIZE      =', TEST_SIZE
    print 'pattern        =', '"'+pattern+'"', len(pattern)
    print 'number hashes  =', num_hashes
    print 'base hashes    =', sorted(set(base_hashes_list))
    print 'rolling hashes =', sorted(set(hashes_list))

cdef arr2str(unsigned char text[], int offset, int length):
    """Convert a segment of a byte array to a Python string"""
    return ''.join([x for x in text[offset:offset+length]])

cdef check_for_patterns(unsigned char text[], int text_len, int pattern_len, 
                unsigned char hash_table[],
                unsigned int offsets[], int num_offsets[]):
    """Check <text> to see if it contains patterns with hash hashes in <hash_table>
        Return offsets of matching text"""

    cdef unsigned int h = _mod(D, pattern_len-1)
    print 'D = %d, pattern_len = %d, h=%d' % (D, pattern_len, h)

    cdef int nofs = 0

    cdef unsigned int t = _get_simple_hash(text, pattern_len, 0) 
       
    for i in range(text_len - pattern_len):
        if hash_table[t % HASH_SIZE] == 1:
            offsets[nofs] = i
            nofs += 1
            #print 'match: %7d - %5d %s' % (t, t % HASH_SIZE, arr2str(text, i, pattern_len))
        t = t - (text[i]*h)
        t = (t * D + text[i+pattern_len]) 
        assert(t == _get_simple_hash(text, pattern_len, i+1))
    
    if hash_table[t % HASH_SIZE]:
        offsets[nofs] = text_len - pattern_len
        nofs += 1

    num_offsets[0] = nofs

def test_check_for_patterns():
    cdef unsigned char text[TEST_SIZE]
    cdef unsigned int offsets[TEST_SIZE]
    cdef int num_offsets[1]
    cdef unsigned char hash_table[HASH_STORAGE]
    
    # Fill text with patterns over background
    background = 'ABCDEFGHIJKLMNOPQRSTRUVWXYZ'
    patterns = ['abcd', 'efgh', 'ijkl', 'mnop', 'qrst', 'uvwx']
    
    for i in range(TEST_SIZE):
        text[i] = ord(background[i %len(background)])
    
    pattern_num = 0
    for i in range(TEST_SIZE//10):
        for j in range(4):
            text[j+i*10] = ord(patterns[pattern_num][j])
        pattern_num = (pattern_num + 1) % len(patterns)

    memset(hash_table, 0, HASH_STORAGE)

    for pat in patterns:
        t = _get_simple_hash(pat, 4, 0)
        hash_table[t % HASH_SIZE] = 1
        print 'hash_table entry:', t % HASH_SIZE

    check_for_patterns(text, TEST_SIZE, 4, hash_table, offsets, num_offsets)

    matches = []
    for i in range(num_offsets[0]):
        matches.append(arr2str(text, offsets[i], 4))
        
    print 'TEST_SIZE      =', TEST_SIZE
    print 'number hashes  =', TEST_SIZE - 3 
    print 'number offsets =', num_offsets[0]
    print 'patterns =', patterns
    print 'matches  =', sorted(set(matches))
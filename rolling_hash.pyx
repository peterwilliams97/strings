from __future__ import division
"""
Rolling hash implementation

Use Cython: http://docs.cython.org/
http://docs.cython.org/src/quickstart/build.html

To build:
python setup.py build_ext --inplace

"""
import common
from common import H, unH

#http://wiki.cython.org/DynamicMemoryAllocation
from libc.stdlib cimport malloc, calloc, free

#http://trac.cython.org/cython_trac/attachment/ticket/314/array.pxd
cdef extern from "stdlib.h" nogil:
    void *memset(void *str, int c, size_t n)
    void *memcpy(void *dst, void *src, size_t n)

DEF INT_SIZE = 4
DEF INT_BITS = INT_SIZE * 8

# Size of the rolling hash table. Smallest 5 digit prime
DEF HASH_SIZE = 100003 #3 100009
# Memory storage for hash table. Round up to multiple of 32 
DEF HASH_STORAGE = 100032

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

cdef _show_stats(unsigned char ht[]):
    count = 0
    for i in range(HASH_SIZE):
        if ht[i]:
            count += 1
    print 'hash stats: %5d of %d = %d%%' % (count, HASH_SIZE, 
        int(100.0 *float(count)/float(HASH_SIZE)))
        
    assert(count > 0)
    
cdef _text_to_string(unsigned char text[], int text_len):
    return ''.join([chr(text[i]) for i in range(text_len)])

_KNOWN_SUBSTRINGS_ALL = '''
H_00000004
H_00000006
H_00000007
H_00000001
H_00000003
H_00040000
H_beef0000
H_01200000
H_05000000
H_ef000000
H_ff02ff02
H_00000400
H_5c800000
H_00000500
H_04000000
H_80000001
H_00000600
H_00050000
H_035c8000
H_00000700
H_000004de
H_40000000 
'''
_KNOWN_SUBSTRINGS = [s.strip() for s in _KNOWN_SUBSTRINGS_ALL.split('\n')]
_KNOWN_SUBSTRINGS = [unH(s) for s in _KNOWN_SUBSTRINGS if s]
_KNOWN_HASHES = [_get_simple_hash(s, len(s), 0) % HASH_SIZE for s in _KNOWN_SUBSTRINGS]

_BAD_SUBSTRING = unH('H_00040000')
_BAD_HASH = _get_simple_hash(_BAD_SUBSTRING, len(_BAD_SUBSTRING), 0) % HASH_SIZE 
   
cdef _test_ht(unsigned char ht[]):
    """Return first memmber of _KNOWN_SUBSTRINGS without a ht entry""" 
    cdef unsigned int t 
    for s in _KNOWN_SUBSTRINGS:
        t = _get_simple_hash(s, len(s), 0)
        if ht[t % HASH_SIZE] == 0:
            return s
        assert(ht[t % HASH_SIZE] != 0)

cdef _get_ht_val(unsigned char ht[], s):
    """Return value in ht of hash of s""" 
    cdef unsigned int t= _get_simple_hash(s, len(s), 0)
    return ht[t % HASH_SIZE] 

cdef _arr2str(unsigned char text[], int offset, int length):
    """Convert a segment of a byte array to a Python string"""
    return ''.join([x for x in text[offset:offset+length]])

cdef _check_for_patterns(unsigned char text[], int text_len, int pattern_len, int min_repeats,
                    unsigned char ht_in[], unsigned char ht_out[]):
    """Check <text> to see if it contains patterns with hash hashes in <hash_table>
        Return offsets of matching text and their hashes"""

    cdef unsigned int h = _mod(D, pattern_len-1)
    #print 'D = %d, pattern_len = %d, h=%d' % (D, pattern_len, h)
    print '_check_for_patterns(text_len=%7d, pattern_len=%3d, repeats=%3d)' % (text_len, 
        pattern_len, min_repeats), 

    cdef unsigned char ht_count[HASH_STORAGE]
    memset(ht_count, 0, HASH_STORAGE)

    bad_hash_count = 0
    bad_hash_count2 = 0
    #print '-' * 40
    cdef unsigned int t = _get_simple_hash(text, pattern_len, 0) 
    for i in range(text_len - pattern_len):
        if t % HASH_SIZE == _BAD_HASH:
            bad_hash_count += 1
            assert(ht_in[t % HASH_SIZE])
            
        if ht_in[t % HASH_SIZE]:
            if ht_count[t % HASH_SIZE] < 254:
                ht_count[t % HASH_SIZE] += 1
            if t % HASH_SIZE == _BAD_HASH:
                #print '  ht_count[t % HASH_SIZE] =', ht_count[t % HASH_SIZE], ',t % HASH_SIZE =', (t % HASH_SIZE) 
                if t % HASH_SIZE == _BAD_HASH:
                    if bad_hash_count2 < 254:
                        bad_hash_count2 += 1
                    #print '  bad_hash_count2 =', bad_hash_count2, ',t % HASH_SIZE =', (t % HASH_SIZE)
                if (ht_count[t % HASH_SIZE] < bad_hash_count2):
                    print ' t =', t
                    print ' _BAD_HASH =', _BAD_HASH
                    print ' HASH_SIZE =', HASH_SIZE
                    print ' t % HASH_SIZE =', (t % HASH_SIZE)
                    print ' ht_count[t % HASH_SIZE] =', ht_count[t % HASH_SIZE]
                    print ' bad_hash_count2 =', bad_hash_count2
                assert(ht_count[t % HASH_SIZE] >= bad_hash_count2)
        t = t - (text[i]*h)
        t = (t * D + text[i+pattern_len]) 
        assert(t == _get_simple_hash(text, pattern_len, i+1))
    if ht_in[t % HASH_SIZE]:
        ht_count[t % HASH_SIZE] += 1

    missed_string = _test_ht(ht_count)
    if missed_string:
        print 'bad ht_count. No', H(missed_string)
        exit()
        
    memset(ht_out, 0, HASH_STORAGE)
    for i in range(HASH_SIZE):
        if ht_count[i] >= min_repeats:
            ht_out[i] = 1

    _show_stats(ht_out)
    missed_string = _test_ht(ht_out)
    if missed_string:
        ht_count_missing = _get_ht_val(ht_count, missed_string)
        print 'bad ht_out. No', H(missed_string)
        print 'bad_hash_count  =', bad_hash_count
        print 'bad_hash_count2 =', bad_hash_count2
        print 'ht_count =', ht_count_missing 
        text_string = _text_to_string(text, text_len)
        offsets = common.get_substring_offsets(text_string, missed_string)
        print 'offsets:', len(offsets), sorted(offsets)
        for offset in offsets:
            s = _arr2str(text, offset, pattern_len)
            assert(s ==  missed_string)
        exit()    

cdef _get_pattern_offsets(unsigned char text[], int text_len, int pattern_len, unsigned char ht_in[]):
    """Return dict of offsets of string that obey min_repeats and whose hashed are in ht_in"""

    cdef unsigned int h = _mod(D, pattern_len-1)
    #print 'D = %d, pattern_len = %d, h=%d' % (D, pattern_len, h)
    print '_get_pattern_offsets(text_len=%7d, pattern_len=%3d)' % (text_len, pattern_len),

    pattern_offsets = {}
  
    cdef unsigned int t = _get_simple_hash(text, pattern_len, 0) 
    for i in range(text_len - pattern_len):
        if ht_in[t % HASH_SIZE]:
            key = _arr2str(text, i, pattern_len)
            if not key in pattern_offsets.keys():
                pattern_offsets[key] = set([])
            pattern_offsets[key].add(i)
        t = t - (text[i]*h)
        t = (t * D + text[i+pattern_len]) 
        #assert(t == _get_simple_hash(text, pattern_len, i+1))

    if ht_in[t % HASH_SIZE]:
        i = text_len - pattern_len
        key = _arr2str(text, i, pattern_len)
        if not key in pattern_offsets.keys():
            pattern_offsets[key] = set([])
        pattern_offsets[key].add(i)
    
    print len(pattern_offsets) if pattern_offsets else pattern_offsets 
    return pattern_offsets

def _trim_pattern_offsets(pattern_offsets, min_count):
    """Remove all pattern_offsets entries with values of less then min_count"""
    
    print '_trim_pattern_offsets(pattern_offsets=%d, min_count=%d)' % (len(pattern_offsets), min_count), 
    
    for key,val in pattern_offsets.items():
        if not val:
            print '++', key, val
        if len(val) < min_count:
            del(pattern_offsets[key])
            
    print len(pattern_offsets)

def _trim_to_common_keys(pattern_offsets_list):
    """Remove all pattern_offsets that in all dicts"""
    print '_trim_to_common_keys(%s) ' % [len(x) for x in pattern_offsets_list] 
    for pattern_offsets in pattern_offsets_list:
        for key in pattern_offsets.keys():
            if not all([p.has_key(key) for p in pattern_offsets_list]):
                del(pattern_offsets[key])
    
    print '    =>', [len(x) for x in pattern_offsets_list]

def get_offsets_from_texts(text_list, min_repeats_list, pattern_len): 
    """Given a list of texts and a list of min repeats for those texts, return
        list of dicts of offsets for which min repeats are satisfied"""

    print 'get_offsets_from_texts(nu texts=%d,pattern_len=%d)' % (len(text_list), pattern_len) 
    # Initialize all the data structurs
    cdef unsigned char ht_in[HASH_STORAGE]
    cdef unsigned char ht_out[HASH_STORAGE]
    
    # Make _check_for_patterns() check all substrings the first time it is called
    memset(ht_in, 1, HASH_STORAGE)

    # Run through the texts once and get a list of pattern_offsets
    pattern_offsets_list = []
    for i,text in enumerate(text_list):
        assert(min_repeats_list[i] <= 255)
        _check_for_patterns(text, len(text), pattern_len, min_repeats_list[i], ht_in, ht_out)
        memcpy(ht_in, ht_out, HASH_SIZE)
        #_trim_pattern_offsets(pattern_offsets, min_repeats_list[i])
        #pattern_offsets_list.append(pattern_offsets)
    
    for i,text in enumerate(text_list):
        pattern_offsets = _get_pattern_offsets(text, len(text), pattern_len, ht_in)
        assert(pattern_offsets)
        _trim_pattern_offsets(pattern_offsets, min_repeats_list[i])
        assert(pattern_offsets)
        pattern_offsets_list.append(pattern_offsets)
    
    # Trim all the pattern_offsets to the common subset 
    _trim_to_common_keys(pattern_offsets_list)
    
    # We now have a list of dicts of offset lists so we can move faster

    return pattern_offsets_list


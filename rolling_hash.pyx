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

# Storing counts in bytes so we need to stop counts from wrapping
DEF _MAX_HASH_COUNT = 250 
# Size of the rolling hash table. Smallest 5 digit prime
DEF _HASH_SIZE = 100003 
# Memory storage for hash table. Round up to multiple of 32 
DEF _HASH_STORAGE = 100032

# The "radix" for the Rabin-Karp rolling hash
# http://en.wikipedia.org/wiki/Rolling_hash
DEF _D = 31

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
        t = (t * _D + text[i+offset]) 
    return t

cdef _get_count(unsigned char ht[]):
    """Return number of non-zero entries in hash table <ht>"""
    cdef int count = 0
    for i in range(_HASH_SIZE):
        if ht[i]:
            count += 1
    return count
    
cdef _show_stats(unsigned char ht[]):
    """Print some stats about hash table <ht>"""
    count = _get_count(ht)
    common.report('hash stats: %5d of %d = %d%%' % (count, _HASH_SIZE, 
        int(100.0 *float(count)/float(_HASH_SIZE))))
    assert(count > 0)

cdef _arr2str(unsigned char text[], int length, int offset):
    """Convert a segment of a byte array to a Python string"""
    return ''.join([x for x in text[offset:offset+length]])   

cdef _get_ht_val(unsigned char ht[], s):
    """Return value of hash of string <s> in hash table <ht>""" 
    cdef unsigned int t= _get_simple_hash(s, len(s), 0)
    return ht[t % _HASH_SIZE] 

cdef _check_for_patterns(unsigned char text[], int text_len, int pattern_len, int min_repeats,
                    unsigned char ht_in[], unsigned char ht_out[]):
    """Check <text> to see if it contains patterns with hash hashes in <hash_table>
        Return offsets of matching text and their hashes"""

    cdef unsigned int h = _mod(_D, pattern_len-1)
    
    common.report('_check_for_patterns(text_len=%7d, pattern_len=%3d, repeats=%3d)' % (text_len, 
        pattern_len, min_repeats)) 

    cdef unsigned char ht_count[_HASH_STORAGE]
    memset(ht_count, 0, _HASH_STORAGE)

    cdef unsigned int t = _get_simple_hash(text, pattern_len, 0) 
    for i in range(text_len - pattern_len):
        if ht_in[t % _HASH_SIZE]:
            if ht_count[t % _HASH_SIZE] < _MAX_HASH_COUNT:
                ht_count[t % _HASH_SIZE] += 1
        t = t - (text[i]*h)
        t = (t * _D + text[i+pattern_len]) 
        #assert(t == _get_simple_hash(text, pattern_len, i+1))
    if ht_in[t % _HASH_SIZE]:
        if ht_count[t % _HASH_SIZE] < _MAX_HASH_COUNT:
            ht_count[t % _HASH_SIZE] += 1
        
    memset(ht_out, 0, _HASH_STORAGE)
    for i in range(_HASH_SIZE):
        if ht_count[i] >= min_repeats:
            ht_out[i] = 1

    _show_stats(ht_out)
    
cdef _get_pattern_offsets(unsigned char text[], int text_len, int pattern_len, unsigned char ht_in[]):
    """Return dict of offsets of string that obey min_repeats and whose hashed are in ht_in"""

    cdef unsigned int h = _mod(_D, pattern_len-1)
    common.report('_get_pattern_offsets(text_len=%7d, pattern_len=%3d)' % (text_len, pattern_len))

    pattern_offsets = {}
  
    cdef unsigned int t = _get_simple_hash(text, pattern_len, 0) 
    for i in range(text_len - pattern_len):
        if ht_in[t % _HASH_SIZE]:
            key = _arr2str(text, pattern_len, i)
            if not key in pattern_offsets.keys():
                # Most time is spent here!! !@#$
                pattern_offsets[key] = set([])
            pattern_offsets[key].add(i)
        t = t - (text[i]*h)
        t = (t * _D + text[i+pattern_len]) 
        #assert(t == _get_simple_hash(text, pattern_len, i+1))

    if ht_in[t % _HASH_SIZE]:
        i = text_len - pattern_len
        key = _arr2str(text, pattern_len, i)
        if not key in pattern_offsets.keys():
            pattern_offsets[key] = set([])
        pattern_offsets[key].add(i)
    
    common.report('pattern_offsets %s'% (len(pattern_offsets) if pattern_offsets else pattern_offsets))
    return pattern_offsets

def _trim_pattern_offsets(pattern_offsets, min_count):
    """Remove all pattern_offsets entries with values of less then min_count"""
    
    common.report('_trim_pattern_offsets(pattern_offsets=%d, min_count=%d)' % (len(pattern_offsets),
            min_count)) 
    
    for key,val in pattern_offsets.items():
        if not val:
            print '++', key, val
        if len(val) < min_count:
            del(pattern_offsets[key])
            
    common.report('pattern_offsets %s' % len(pattern_offsets))

def _trim_to_common_keys(pattern_offsets_list):
    """Remove all pattern_offsets that in all dicts"""
    common.report('_trim_to_common_keys(%s) ' % [len(x) for x in pattern_offsets_list]) 
    for pattern_offsets in pattern_offsets_list:
        for key in pattern_offsets.keys():
            if not all([p.has_key(key) for p in pattern_offsets_list]):
                del(pattern_offsets[key])
    common.report('    => %s' % [len(x) for x in pattern_offsets_list])


def get_offsets_from_texts(text_list, min_repeats_list, pattern_len): 
    """Given a list of texts and a list of min repeats for those texts, return
        list of dicts of offsets for which min repeats are satisfied"""

    common.note_time('get_offsets_from_texts(num texts=%d,pattern_len=%d)' % (len(text_list), 
        pattern_len)) 
    # Initialize all the data structurs
    cdef unsigned char ht_in[_HASH_STORAGE]
    cdef unsigned char ht_out[_HASH_STORAGE]
    
    # Make _check_for_patterns() check all substrings the first time it is called
    memset(ht_in, 1, _HASH_STORAGE)

    pattern_offsets_list = []

    # Run through the texts once and get a list of pattern_offsets
   
    for i,text in enumerate(text_list):
        assert(min_repeats_list[i] <= _MAX_HASH_COUNT)
        _check_for_patterns(text, len(text), pattern_len, min_repeats_list[i], ht_in, ht_out)
        memcpy(ht_in, ht_out, _HASH_SIZE)
        #_trim_pattern_offsets(pattern_offsets, min_repeats_list[i])
        #pattern_offsets_list.append(pattern_offsets)
        common.note_time('_check_for_patterns %2d:len=%7d' % (i, len(text)))

    for i,text in enumerate(text_list):
        pattern_offsets = _get_pattern_offsets(text, len(text), pattern_len, ht_in)
        #assert(pattern_offsets)
        _trim_pattern_offsets(pattern_offsets, min_repeats_list[i])
        #assert(pattern_offsets)
        pattern_offsets_list.append(pattern_offsets)
        if not pattern_offsets:
            break
        common.note_time('_get_pattern_offsets %2d:len=%7d patterns=%d' % (i, len(text), 
            len(pattern_offsets)))

    # Trim all the pattern_offsets to the common subset 
    _trim_to_common_keys(pattern_offsets_list)
    common.note_time('_trim_to_common_keys %d' % len(pattern_offsets_list[0]))
    
    # We now have a list of dicts of offset lists so we can move faster
    return pattern_offsets_list

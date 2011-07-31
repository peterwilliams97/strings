"""
Rolling hash implementation
Cython: http://docs.cython.org/

http://docs.cython.org/src/quickstart/build.html
python setup.py build_ext --inplace

"""
#from libc.stdlib cimport memset

#http://wiki.cython.org/DynamicMemoryAllocation
from libc.stdlib cimport malloc, calloc, free

#http://trac.cython.org/cython_trac/attachment/ticket/314/array.pxd
cdef extern from "stdlib.h" nogil:
    void *memset(void *str, int c, size_t n)

#def say_hello_to(name):
#    print("Hello %s!" % name)

DEF NUMBER_PRIMES = 6
DEF TABLE_SIZE = 6

# Size of the rolling hash table. Smallest 4 digit prime
DEF HASH_SIZE = 1009
cdef unsigned char hash_table[TABLE_SIZE]

# The base of the numeral system
DEF H = 31
# A big enough prime number
DEF Q = HASH_SIZE

# http://harvestsoft.net/rabinkarp.
cdef rabin_karp(char text[], int text_len, char pattern[], int pattern_len,
                unsigned char hits[]):

    memset(hits, 0, text_len)
    
    if text_len < pattern_len:
        return                      # no match is possible

    cdef unsigned int p = 0         # the hash value of the pattern
    cdef unsigned int t = 0         # the hash value of the text

    for i in range(pattern_len): 
        p = (p * H + pattern[i]) % Q
        t = (t * H + text[i]) % Q

    # start the "rolling hash" - for every next character in
    for i in range(text_len - pattern_len):
        if t == p:
            hits[i] = 1
        t = (t *( t - ((text[i]*H) % Q)) + text[i+pattern_len]) % Q

#http://docs.cython.org/src/userguide/external_C_code.html
ctypedef struct String:
    int length
    char * data
    
cdef get_possible_hits(char text[], int text_len, 
                String patterns[], int num_patterns,
                unsigned char hits[]):

    memset(hits, 0, text_len)
    cdef unsigned char pattern_hash_table[HASH_SIZE]
    memset(pattern_hash_table, 0, sizeof(pattern_hash_table))

    if num_patterns <= 0:
        return

    # For efficiency all patterns are same length !@#$
    pattern_len =  patterns[0].length
    if text_len < pattern_len:
        return                              # no match is possible

    #cdef unsigned int p[num_patterns]      # the hash values of the patterns
    #memset(p, 0, sizeof(p))
    
    cdef unsigned int *p = <unsigned int *>calloc(num_patterns, sizeof(unsigned int))
    cdef unsigned int t = 0                 # the hash value of the text

    for j in range(num_patterns):
        for i in range(pattern_len): 
            p[j] = (p[j] * H + patterns[j].data[i]) % Q  
        pattern_hash_table[p[j] % HASH_SIZE] = 1    

    for i in range(text_len): 
        t = (t * H + text[i]) % Q

    # start the "rolling hash" - for every next character in
    for i in range(text_len - patterns[0].length):
        if pattern_hash_table[t % HASH_SIZE]:
            for j in range(num_patterns):
                if t == p[j]:
                    hits[i] = 1
        t = (t *( t - ((text[i]*H) % Q)) + text[i+pattern_len]) % Q       



    
    
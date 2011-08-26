from __future__ import division
"""
A sparse string class

TODO: Allow sparsifying of sparse strings
"""
import sys
import glob
import re
import os
import time
import random
import pickle

class GeneralString:
    """Common interface for python str and SparseStrings"""
    def __init__(self, string):
        self._string = string
        self._is_sparse = isinstance(self._string, SparseString)
        
    def get(self, offset):
        """Return character at <self._string>[offset"""
        if self._is_sparse:
            return self._string.get(offset)
        else:
            return self._string[offset]
        
        
    def get_interval(self, offset0, offset1): 
        """Return string at <self._string>[offset0:offset1]"""
        if self._is_sparse:
            return self._string.get_interval(offset0, offset1)
        else:
            return self._string[offset0:offset1]
    

_CHUNK_SIZE = 16
_PAD = 8

print '_CHUNK_SIZE', _CHUNK_SIZE

def _chunk_before(offset):
    return max(0, offset)//_CHUNK_SIZE
    
def _chunk_after(offset, max_len):
    return min(max_len, offset+ _CHUNK_SIZE - 1)//_CHUNK_SIZE

class SparseString:
    """String with holes in it"""
    def __init__(self, string):
        self._chunks = set([])
        self._max_len = len(string)
        self._string = string

    def add_interval(self, offset0, offset1):
        """Add an interval of offsets that are to be accessible in the sparse version of the string"""
        for chunk in range(_chunk_before(offset0 - _PAD), _chunk_after(offset1 + _PAD, self._max_len)):
            self._chunks.add(chunk)

    def add(self, offset):
        """Add an offset that are to be accessible in the sparse version of the string"""
        self._add_interval(offset, offset+1)        

    def sparsify(self):
        """Replace self._string with self._fragments and self._fragment_map
            Coallesce continguous chunks in doing so"""
        if self._chunks:
            self._fragments = []
            self._fragment_map = {}
            chunk_list = sorted(self._chunks)
            
            chunk = start_chunk = chunk_list[0]
            self._fragment_map[chunk] = len(self._fragments)
            end_chunk = start_chunk + 1     
            for chunk in chunk_list[1:]:
                if chunk not in self._chunks:
                    self._fragments.append(self._string[start_chunk * _CHUNK_SIZE: end_chunk * _CHUNK_SIZE])
                    start_chunk = chunk
                    self._fragment_map[chunk] = len(self._fragments)
                    end_chunk = start_chunk +1
                    print 'a:end_chunk', end_chunk
                else:
                    self._fragment_map[chunk] = len(self._fragments)
                    end_chunk = chunk +1
                    print 'b:end_chunk', end_chunk
                print chunk, start_chunk, end_chunk, len(self._fragments)
            print 'c:end_chunk', end_chunk
            self._fragments.append(self._string[start_chunk * _CHUNK_SIZE: end_chunk * _CHUNK_SIZE])
            
        self._string = None
        print '_chunks', self._chunks
        print '_fragment_map', self._fragment_map
        print '_fragments', ['%d:%s' % (len(f),f) for f in self._fragments]
        #exit()
    
    def _get_first_chunk(self, offset):
        """Return first chunk in the range of contiguous chunks containing <offset>"""
        return self._fragment_map[_chunk_before(offset)]
        
    def get(self, offset):
        """Return character at <offset> in <self._string>"""
        chunk = self._get_first_chunk(offset)
        #print 'get(%3d)' % offset, (chunk_major, chunk_minor), '%3d' % (offset - chunk_minor * _CHUNK_SIZE), 
        val = self._fragments[chunk][offset - chunk * _CHUNK_SIZE]
        #print val,
        return val
        
    def get_interval(self, offset0, offset1): 
        """Return string in <self._string>[offset0:offset1]"""
        chunk = self._get_first_chunk(offset0)
        return self._fragments[chunk][offset0 - chunk * _CHUNK_SIZE: offset1 - chunk * _CHUNK_SIZE]

if __name__ == '__main__':
    interval = _CHUNK_SIZE * 3
    gap = _CHUNK_SIZE // 4
    total_size = interval * 5
    
    base = 'abcdefgh'
    line = 'X'.join([base for i in range(interval//len(base))])
    test_string = 'Z'.join([line for i in range(total_size//len(line))])
    
    print 'test_string = ', len(test_string), test_string
    
    sparse_string = SparseString(test_string)
    
    interval = _CHUNK_SIZE * 10
    gap = _CHUNK_SIZE // 8
    
    offset_list = [offset for offset in range(len(test_string) - interval)]
    print 'offset_list =', len(offset_list), offset_list
        
    for offset in offset_list:
        sparse_string.add_interval(offset, offset+gap)
        
    sparse_string.sparsify()  

    gen_string1 = GeneralString(test_string)
    gen_string2 = GeneralString(sparse_string)    
    
    for offset in offset_list:
        v1 = test_string[offset]
        v2 = sparse_string.get(offset)
        #print '-- %3d' % offset, v1, v2
        assert(v1 == v2)
        v3 = gen_string1.get(offset)
        v4 = gen_string2.get(offset)
        assert(v2 == v3)
        assert(v3 == v4)

    for offset in range(len(test_string) - interval):
        v1 = test_string[offset:offset+gap]
        v2 = sparse_string.get_interval(offset, offset + gap)
        #print '-- %3d' % offset, v1, v2
        assert(v1 == v2)
        v3 = gen_string1.get_interval(offset, offset + gap)
        v4 = gen_string2.get_interval(offset, offset + gap)
        assert(v2 == v3)
        assert(v3 == v4)
        

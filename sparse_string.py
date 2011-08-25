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

_CHUNK_SIZE = 16
_PAD = 8

print '_CHUNK_SIZE', _CHUNK_SIZE

def _chunk_before(offset):
    return max(0, offset)//_CHUNK_SIZE
    
def _chunk_after(offset, max_len):
    return min(max_len, offset+ _CHUNK_SIZE - 1)//_CHUNK_SIZE

class SparseString:
    def __init__(self, string):
        self._chunks = set([])
        self._max_len = len(string)
        self._string = string
    
    def add(self, offset0, offset1):
        #print 'add(%4d,%4d)' % (offset0, offset1)
        for chunk in range(_chunk_before(offset0 - _PAD), _chunk_after(offset1 + _PAD, self._max_len)):
            self._chunks.add(chunk)
            
    def coallesce(self):
        if self._chunks:
            self._fragments = []
            self._fragment_map = {}
            chunk_list = sorted(self._chunks)
            
            chunk = start_chunk = chunk_list[0]
            self._fragment_map[chunk] = (len(self._fragments), 0)
            end_chunk = start_chunk + 1     
            for chunk in chunk_list[1:]:
                if chunk not in self._chunks:
                    self._fragments.append(self._string[start_chunk * _CHUNK_SIZE: end_chunk * _CHUNK_SIZE])
                    start_chunk = chunk
                    self._fragment_map[chunk] = (len(self._fragments), 0)
                    end_chunk = start_chunk +1
                    print 'a:end_chunk', end_chunk
                else:
                    self._fragment_map[chunk] = (len(self._fragments), chunk - start_chunk)
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
    
    def _get_entry(self, offset):
        chunk = _chunk_before(offset)
        return self._fragment_map[chunk]
        
    def get(self, offset):
        chunk_major, chunk_minor = self._get_entry(offset)
        chunk_minor = chunk_major
        print 'get(%3d)' % offset, (chunk_major, chunk_minor), '%3d' % (offset - chunk_minor * _CHUNK_SIZE), 
        val = self._fragments[chunk_major][offset - chunk_minor * _CHUNK_SIZE]
        print val,
        return val
        
    def get_interval(self, offset0, offset1): 
        chunk_major, chunk_minor = self._get_entry(offset0)
        chunk_minor = chunk_major
        return self._fragments[chunk_major][offset0 - chunk_minor * _CHUNK_SIZE: offset1 - chunk_minor * _CHUNK_SIZE]

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
        sparse_string.add(offset, offset+gap)
        
    sparse_string.coallesce()    
    
    for offset in offset_list:
        v1 = test_string[offset]
        v2 = sparse_string.get(offset)
        print '-- %3d' % offset, v1, v2
        assert(v1 == v2)
        
    for offset in range(len(test_string) - interval):
        v1 = test_string[offset:offset+gap]
        v2 = sparse_string.get_interval(offset, offset + gap)
        print '-- %3d' % offset, v1, v2
        assert(v1 == v2)
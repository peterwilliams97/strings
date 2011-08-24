from __future__ import division
"""
A sparse string class
"""
import sys
import glob
import re
import os
import time
import random
import pickle

_CHUNK_SIZE = 1024
_PAD = 512

def _chunk_before(offset):
    return max(0, offset - _PAD)//_CHUNK_SIZE
    
def _chunk_after(offset, max_len):
    return min(max_len, offset + _PAD + _CHUNK_SIZE - 1)//_CHUNK_SIZE

class SparseString:
    def __init__(self, string):
        self._chunks = set([])
        self._max_len = len(string)
        self._string = string
    
    def add(self, offset0, offset1):
        #print 'add(%4d,%4d)' % (offset0, offset1)
        for chunk in range(_chunk_before(offset0), _chunk_after(offset1, self._max_len)):
            self._chunks.add(chunk)
            
    def coallesce(self):
        if self._chunks:
            self._fragments = []
            self._fragment_map = {}
            chunk_list = sorted(self._chunks)
            start_chunk = chunk_list[0]
            self._fragment_map[start_chunk] = (len(self._fragments), 0)
            end_chunk = start_chunk + 1     
            for chunk in chunk_list[1:]:
                if chunk not in self._chunks:
                    self._fragments.append(self._string[start_chunk * _CHUNK_SIZE: end_chunk * _CHUNK_SIZE])
                    start_chunk = chunk
                    self._fragment_map[chunk] = (len(self._fragments), 0)
                    end_chunk = start_chunk + 1 
                else:
                    self._fragment_map[chunk] = (len(self._fragments), chunk - start_chunk)
                    end_chunk = chunk
                print chunk, start_chunk, end_chunk, len(self._fragments)
            self._fragments.append(self._string[start_chunk * _CHUNK_SIZE: end_chunk * _CHUNK_SIZE])
        self._string = None
        print '_chunks', self._chunks
        print '_fragment_map', self._fragment_map
        print '_fragments', [len(f) for f in self._fragments]
    
    def _get_entry(self, offset):
        chunk = _chunk_before(offset)
        return self._fragment_map[chunk]
        
    def get(self, offset):
        entry = self._get_entry(offset)
        val = self._fragments[entry[0]][offset - entry[1] * _CHUNK_SIZE]
        print 'get(%d)' % offset, entry, val
        return val
        
    def get_interval(self, offset0, offset1): 
        entry = self._get_entry(offset0)
        return self._fragments[entry[0]][offset0 - entry[1] * _CHUNK_SIZE: offset1 - entry[1] * _CHUNK_SIZE]

if __name__ == '__main__':
    line = ' '.join(['abcdefghijkl' for i in range(10)])
    test_string = '\n'.join([line for i in range(100)])
    sparse_string = SparseString(test_string)
    
    interval = 5000
    gap = 10
    for offset in range(len(test_string) - interval):
        sparse_string.add(offset, offset+gap)
        
    sparse_string.coallesce()    
    
    for offset in range(len(test_string) - interval):
        v1 = test_string[offset]
        v2 = sparse_string.get(offset)
        print 't:', offset, v1, v2
        assert(v1 == v2)
        
    for offset in range(len(test_string) - interval):
        assert(sparse_string.get_interval(offset, offset + gap) == test_string[offset:offset+gap])    
    
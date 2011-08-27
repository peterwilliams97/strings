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

def GeneralString(string):
    return  string
    
class was_GeneralString:
    """Common interface for python str and SparseStrings"""
    def __init__(self, string):
        self._string = string
        self._is_python_str = isinstance(self._string, str)
        #print 'self._is_python_str', self._is_python_str
        
    def get_len(self):
        if not self._is_python_str:
            return self._string.get_len()
        else:
            #print 'self._string', self._string
            return len(self._string)
        
    def get(self, offset):
        """Return character at <self._string>[offset"""
        if not self._is_python_str:
            return self._string.get(offset)
        else:
            return self._string[offset]

    def get_interval(self, offset0, offset1): 
        """Return string at <self._string>[offset0:offset1]"""
        if not self._is_python_str:
            return self._string.get_interval(offset0, offset1)
        else:
            return self._string[offset0:offset1]
    
_CHUNK_SIZE= 1024
_PAD = 128

print '_CHUNK_SIZE', _CHUNK_SIZE

def _chunk_before(offset):
    return max(0, offset)//_CHUNK_SIZE
    
def _chunk_after(offset, max_chunk):
    return min(max_chunk, (offset + _CHUNK_SIZE - 1)//_CHUNK_SIZE)

class SparseString:
    """String with holes in it"""
    def __init__(self, string):
        self._chunks = set([])
        self._string = string #GeneralString(string)
        self._max_len = len(self._string) #self._string.get_len()
        self._max_chunk = (self._max_len + _CHUNK_SIZE - 1)//_CHUNK_SIZE
        
    def __repr__(self):
        if self._string:
            return self._string.__repr__()
        else:
            return self._fragment_map.__repr__()
        
    def __len__(self):
        return self._max_len
        
    def __getitem__(self, key):
        if self._string:
            return self._string.__getitem__(key)
        elif isinstance(key, int):
            return self.get(key)
        elif isinstance(key, slice):
            #print 'key =', key
            return self.get_interval(key.start, key.stop)
        
    def get_len(self):
        return self._max_len

    def add_interval(self, offset0, offset1):
        """Add an interval of offsets that are to be accessible in the sparse version of the string"""
        #print 'add_interval(%d, %d)' % (offset0, offset1),
        for chunk in range(_chunk_before(offset0 - _PAD), _chunk_after(offset1 + _PAD, self._max_chunk)):
            #print chunk,
            self._chunks.add(chunk)
        #print
        #print 'before, after =', _chunk_before(offset0 - _PAD), _chunk_after(offset1 + _PAD, self._max_chunk)
        assert(self._chunks)
            
    def add(self, offset):
        """Add an offset that are to be accessible in the sparse version of the string"""
        self._add_interval(offset, offset+1)        

    def sparsify(self):
        """Replace self._string with self._fragments and self._fragment_map
            Coallesce continguous chunks in doing so"""
        assert(self._chunks)    
        if self._chunks:
            self._fragments = []
            self._fragment_map = {}
            chunk_list = sorted(self._chunks)
            
            chunk = start_chunk = chunk_list[0]
            self._fragment_map[chunk] = len(self._fragments)
            end_chunk = start_chunk + 1     
            for chunk in chunk_list[1:]:
                if chunk not in self._chunks:
                    self._fragments.append(self._stringget_interval(start_chunk * _CHUNK_SIZE, end_chunk * _CHUNK_SIZE))
                    start_chunk = chunk
                    self._fragment_map[chunk] = len(self._fragments)
                    end_chunk = start_chunk +1
                    #print 'a:end_chunk', end_chunk
                else:
                    self._fragment_map[chunk] = len(self._fragments)
                    end_chunk = chunk +1
                    #print 'b:end_chunk', end_chunk
                #print chunk, start_chunk, end_chunk, len(self._fragments)
            #print 'c:end_chunk', end_chunk
            self._fragments.append(self._string[start_chunk * _CHUNK_SIZE:end_chunk * _CHUNK_SIZE])
            #self._fragments.append(self._string.get_interval(start_chunk * _CHUNK_SIZE, end_chunk * _CHUNK_SIZE))
            
            self._string = None
        else:
            raise Exception
        if False:
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

def _base_test(test_string, sparse_string, gap, offset_list):

    general_string = GeneralString(test_string)    
    gen_string1 = GeneralString(test_string)
    gen_string2 = GeneralString(sparse_string) 
    
    for offset in offset_list:
        v1 = GeneralString(test_string)[offset]
        v2 = sparse_string[offset]
        #print '-- %3d' % offset, v1, v2
        assert(v1 == v2)
        v3 = gen_string1[offset]
        v4 = gen_string2[offset]
        assert(v2 == v3)
        assert(v3 == v4)

    for offset in offset_list:
        v1 = GeneralString(test_string)[offset : offset + gap]
        v2 = sparse_string[offset : offset + gap]
        #print '-- %3d' % offset, v1, v2
        assert(v1 == v2)
        v3 = gen_string1[offset : offset + gap]
        v4 = gen_string2[offset : offset + gap]
        assert(v2 == v3)
        assert(v3 == v4)
        
    #print '-' * 80        
        
def _test(test_string, gap, offset_list):
      
    sparse_string = SparseString(test_string)
     
    for offset in offset_list:
        sparse_string.add_interval(offset, offset+gap)
        
    sparse_string.sparsify()  
    
    _base_test(test_string, sparse_string, gap, offset_list)
    
def _test_general_string(test_string, gap, offset_list):
      
    sparse_string = SparseString(test_string)
     
    for offset in offset_list:
        sparse_string.add_interval(offset, offset+gap)
        
    sparse_string.sparsify()  
    test_string = GeneralString(sparse_string) 
    sparse_string = SparseString(test_string)
     
    for offset in offset_list:
        sparse_string.add_interval(offset, offset+gap)
        
    sparse_string.sparsify()  
    
    _base_test(test_string, sparse_string, gap, offset_list)    

def _offsets_test(interval, gap):
    total_size = interval * 5
    
    base = 'abcdefgh'
    line = 'X'.join([base for i in range(interval//len(base))])
    test_string = 'Z'.join([line for i in range(total_size//len(line))])
    
    print '_offsets_test(interval=%d, gap=%d) test_string = %d' % (interval, gap, len(test_string))
 
    offset_list = [offset for offset in range(0, (len(test_string) - gap), interval)]
    #print 'offset_list =', len(offset_list), offset_list
    _test(test_string, gap, offset_list)
    _test_general_string(test_string, gap, offset_list)

    offset_list = [offset for offset in range(len(test_string) - interval)]
    #print 'offset_list =', len(offset_list), offset_list
    _test(test_string, gap, offset_list)
    _test_general_string(test_string, gap, offset_list)
    
def _interval_gap_test():
    interval = _CHUNK_SIZE * 10
    gap = _CHUNK_SIZE // 8
    _offsets_test(interval, gap)
    
    interval = _CHUNK_SIZE * 3
    gap = _CHUNK_SIZE // 4
    _offsets_test(interval, gap)
    
    interval = _CHUNK_SIZE * 99
    gap = _CHUNK_SIZE // 8
    _offsets_test(interval, gap)
   
def _chunk_size_test():
    global _CHUNK_SIZE
    global _PAD
    
    chunk_size = _CHUNK_SIZE
    pad = _PAD
    
    _CHUNK_SIZE = 16
    _PAD = 8
    _interval_gap_test()
    
    _CHUNK_SIZE = chunk_size 
    _PAD = pad
    _interval_gap_test()    
    
def _memory_test(num_rounds):
    interval = 1024 * 1024
    gap = 10 * 1024
    total_size = interval * 100
    
    base = 'abcdefghi'
    line = 'X'.join([base for i in range(interval//len(base))])
    test_string = 'Z'.join([line for i in range(total_size//len(line))])
    line = None
    print 'test_string', len(test_string)
        
    offset_list = [offset for offset in range(0, (len(test_string) - gap), interval*11)]
    offset_list = [0]
    for i in range(num_rounds):
        print 'sparsifying:', i
        sparse_string = SparseString(test_string)
        for offset in offset_list:
            sparse_string.add_interval(offset, offset+gap)
        sparse_string.sparsify() 
        test_string = GeneralString(sparse_string) 
        
def _test_special_funcs(obj, key):
    print '_test_special(obj=%s,key=%s)' % (obj,key)
    print '%s.__repr__() = %s' % (obj, obj.__repr__())
    print '%s.__len__() = %d' % (obj, obj.__len__())
    print '%s.__getitem__(%s) = %s' % (obj, key, obj.__getitem__(key))
        
def _test_special():
    test_string = 'hello'
    sparse_string = SparseString(test_string)
    
    _test_special_funcs(test_string, 0)
    _test_special_funcs(sparse_string, 0)
    
    sparse_string.add_interval(2, 4)
    sparse_string.sparsify()
    
    _test_special_funcs(sparse_string, 0)
    print 'sparse_string[2:5]', sparse_string[2:5]
    
if __name__ == '__main__':
    _chunk_size_test()
    #_memory_test(1024 * 1024)
    _test_special()
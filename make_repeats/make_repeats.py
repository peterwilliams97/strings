from __future__ import division
"""
Make test files for https://github.com/peterwilliams97/strings/tree/master/repeats/repeats
"""
import random

REPEATED_STRING = 'repeated'

def make_repeats(size, num_repeats):
    repeat_size = size//num_repeats
    data = []
    for i in range(size):
        data.append(random.randint(ord('a'),ord('z')))
        
    for i in range(num_repeats):
        for j in range(len(REPEATED_STRING)):
            data[i*repeat_size+j] = ord(REPEATED_STRING[j])    
        
    return ''.join([chr(x) for x in data]) 

def make_repeats_safe(size, num_repeats):
    for i in range(10):
        text = make_repeats(size, num_repeats)
        actual_repeats = text.count(REPEATED_STRING)
        if actual_repeats == num_repeats:
            return text
        print 'Repeats mismatch! actual_repeats=%d,num_repeats=%d' % (
            actual_repeats, num_repeats)
    assert false, 'Cannot be here'

    
def make_repeats_file(size, num_repeats):
    name = 'repeats=%d.txt' % num_repeats
    print 'make_repeats_file(%d, %d) name="%s"' % (size, num_repeats, name)
    file(name, 'wb').write(make_repeats_safe(size, num_repeats))

MBYTE = 1024 * 1024    
def main():
    # The Nelson! 
    random.seed(111)
    
    for num_repeats in range(1,21):
        make_repeats_file(MBYTE, num_repeats)
        
main()

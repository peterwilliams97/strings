from __future__ import division
"""
Make test files for https://github.com/peterwilliams97/strings/tree/master/repeats/repeats
"""
import random, os

REPEATED_STRING = '"the repeated string"'

def make_repeats(size, num_repeats):
    repeat_size = size//num_repeats
    data = []
    
    # Random lower case letters
    for i in range(size):
        data.append(random.randint(ord('a'),ord('z')))
        
    n = num_repeats % 36
    if n < 10:
        c = ord('0') + n
    else:
        c = ord('A') + n - 10

    # One number or uppercase letter specific to file    
    N = size - 1     
    for i in range(max(num_repeats * 2, size // 100)):        
        data[random.randint(0, N)] = c
    
    # Some values that occur onece
    for i in range(10):
        data[len(REPEATED_STRING) + i] = ord('!') + i 
    
    # The repeated string     
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
    return os.path.abspath(name).replace('\\', '\\\\')

KBYTE = 1024
MBYTE = KBYTE ** 2
GBYTE = KBYTE ** 3

def main():
    # The Nelson! 
    random.seed(111)
    
    entries = []
    for num_repeats in range(11,21):
        path = make_repeats_file(10 * MBYTE, num_repeats)
        entries.append('    { %2d, string("%s") }' % (num_repeats, path)) 
    
    print 'entries[%d] = {\n' % len(entries) + ',\n'.join(entries) + '\n};'     
        
main()


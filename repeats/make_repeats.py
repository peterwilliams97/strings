from __future__ import division
"""
Make test files for https://github.com/peterwilliams97/strings/tree/master/repeats/repeats
"""
import random, os

REPEATED_STRING = 'the long long long repeated string'
#REPEATED_STRING = '0123456789'

OTHER_STRINGS = [
    'Finland', 'Sweden', 'Pakistan', 'India', 'Doctor'
    'poodle', 'spaniel', 'modern', 'mythical', 'orginal',
    'ancient', '1000 cities', '100 nights', '10 fathers',
    'Mercury', 'Venus', 'Earth', 'Mars', 'Jupiter', 
    'Saturn', 'Uranus', 'Neptune', 'Pluto'
]

def make_repeats(size, num_repeats, method):
    repeat_size = size//num_repeats
    data = []
    
    if method == 0:
        # Random lower case letters
        for i in range(size):
            data.append(random.randint(ord('a'),ord('z')))
    
    elif method == 1:    
        for i in range(size):
            data.append(random.randint(0, 255))
            
    elif method == 2:    
        for i in range(size//4):
            data.append(random.randint(ord('a'),ord('z')))
            data.append(random.randint(ord('A'),ord('Z')))
            data.append(random.randint(ord('0'),ord('9')))
            data.append(random.randint(10, 255))
     
    elif method == 3:    
        for i in range(size//3):
            data.append(random.randint(ord('a'),ord('z')))
            data.append(random.randint(ord('A'),ord('Z')))
            data.append(random.randint(ord('0'),ord('9')))
            
    elif method == 4:    
        for i in range(size):
            data.append(random.randint(ord('A'),ord('Z')))
     
    n = num_repeats % 36
    if n < 10:
        c = ord('0') + n
    else:
        c = ord('A') + n - 10

    # One number or uppercase letter specific to file    
    N = size - 1     
    for i in range(max(num_repeats * 2, size // 100)):        
        data[random.randint(0, N)] = c
    
    # Some values that occur once
    for i in range(10):
        data[len(REPEATED_STRING) + i] = ord('!') + i 
    
    # The other strings    
    if False:
        for i in range(num_repeats*3):
            for s0 in OTHER_STRINGS:
                s = ' ' + s0 + ' '  
                p = random.randint(0, N - len(s))
                for j in range(len(s)):
                    data[p+j] = ord(s[j])
            
    # The repeated string     
    for i in range(num_repeats):
        for j in range(len(REPEATED_STRING)):
            data[i*repeat_size+j] = ord(REPEATED_STRING[j]) 
        
    result = ''.join([chr(x) for x in data]) 
    assert result.count(REPEATED_STRING) == num_repeats
    return result

def make_repeats_file(directory, size, num_repeats, method):
    path = os.path.join(directory, 'repeats=%d.txt' % num_repeats)
    print 'make_repeats_file(%d, %d) name="%s"' % (size, num_repeats, path)
    file(path, 'wb').write(make_repeats(size, num_repeats, method))
    return os.path.abspath(path).replace('\\', '\\\\')

KBYTE = 1024
MBYTE = KBYTE ** 2
GBYTE = KBYTE ** 3

def main():
    # The Nelson! 
    random.seed(111)
    
    import optparse, sys

    parser = optparse.OptionParser('python ' + sys.argv[0] + ' [options]')
    parser.add_option('-r', '--min-repeats', dest='min', default='11',  help='min num of repeats')
    parser.add_option('-n', '--number', dest='num', default='5', help='number of documents')
    parser.add_option('-s', '--size', dest='size', default='1.0', help='size of each document in MBytes')
    parser.add_option('-m', '--method', dest='method', default='1', help='Method used to documents')
    parser.add_option('-d', '--directory', dest='directory', default='.', help='Directory to create file in')

    (options, args) = parser.parse_args()

    size = int(float(options.size)*MBYTE)
    min_repeats = int(options.min)
    num_documents = int(options.num)
    method = int(options.method)
    directory = options.directory

    print 'size = %.3f Mbyte' % (size/MBYTE)
    print 'method = %d' % method 
    print 'min_repeats = %d' % min_repeats 
    print 'num_documents = %d' % num_documents  
    print 'directory = "%s"' % directory  
    print '-' * 80
    
    entries = []
    for num_repeats in range(min_repeats, min_repeats + num_documents):
        
        path = make_repeats_file(directory, size, num_repeats, method)
        entries.append('    { %2d, string("%s") }' % (num_repeats, path)) 

    print '-' * 80    
    print 'entries[%d] = {\n' % len(entries) + ',\n'.join(entries) + '\n};'     

main()


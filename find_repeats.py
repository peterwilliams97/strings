from __future__ import division
"""
    Script to find the longest string(s) that is repeated M times in a corpus 
    of test files where M is generally different for each file.

    This script works by 
        - finding all <n>grams (strings of length n) that are repeated M or
            more times for each file in the corpus (M generally depends on 
            file). These are called the valid <n>grams (ngrams in the code). 
        - constructing the <n+1>grams consistent with the valid <n>grams. 
            There can be no more than 512 as many <n+1>grams as <n>grams.
        - repeating for n <- n+1 util there are no valid <n+1> grams

    This script was written to be efficient for a particular corpus of files. 
    There is no reason to believe that it will be efficient for all corpora.

    This script worked for a corpus where
        - there was no human readable high level structure identifing the M
            repeats. Hence the need for a brute force method like this.
        - There were a small (<30%) fraction of valid 1grams and 2grams. 
            This substantially reduced the number of strings that needed to 
            be checked.
            
    Keys to performance:
        1. A corpus that has a low fraction of valid 1grams and 2grams.
        2. Innermost loop (see sufficient(s) in code below) is fast because
          a) Python text.count(s) is fast 
          b) all(text.count(s) >= numpages for ...) terminates on first False
        1 and 2a) together give most of the performance 
"""

import re, glob, os

def mb(b):
    """Return number of megabytes in b bytes"""
    return b/1024.0/1024.0
       
def get_files(file_pattern):
    """Simple glob. Return files matching file_pattern"""
    return [p for p in glob.glob(file_pattern) if not os.path.isdir(p)]    

# Distance to either side of a page boundary retained in compress()    
RANGE = 1024*1024       

def compress(text, numpages):
    """Markers are likely to be near start of end of pages
        We simplistically assume pages of roughly equal size 
        and use only the data within RANGE of multiples of 
        size of file/numbers
    """
    # Offsets of for equally sized pages
    breaks = [int(len(text) * n/numpages) for n in range(numpages)] + [len(text)]
    
    # Boundaries of RANGE bytes before and after each break
    boundaries = [[max(0,i-RANGE),min(len(text),i+RANGE)] for i in breaks]
    for i in range(1, len(boundaries)):
        boundaries[i][0] = max(boundaries[i][0], boundaries[i-1][1])
    boundaries = [(s,f) for (s,f) in boundaries if f > s]  
    
    # Return the data within the boundaries    
    return ''.join([text[s:f] for (s,f) in boundaries])

def get_numpages(filename):
    """Return number of pages indicated by our file naming convention
        Exit if filename does not match convention
    """
    m = re.search(r'pages(\d+)', filename)
    assert m, '%s does not have required format' % filename
    return int(m.group(1))    

def get_data(filename):
   """Return (filename,numpages,text) for filename
        text is the compressed contents of the file
   """
   numpages = get_numpages(filename)
   text = compress(file(filename, 'rb').read(), numpages)
   return filename, numpages, text
   
def analyze(path_list):
    """Find longest string that is repeated >= numpages times for all files 
        in the corpus in path_list.
        Path list is a list of spool files with their number of pages encoded
        as pages<numpages>
        e.g. if path_list = ['excel_pages2.prn', 'mixed_junk_pages4.prn']
              then this function will return the longest string that is repeated
                >= 2 times in excel_pages2.prn and
                >= 4 times in mixed_junk_pages4.prn
    """    
    
    corpus = [get_data(filename) for filename in path_list]
    
    # Display order is by numpages then filename
    display_order = lambda x:(x[1],x[0])
    
    # Show the file list
    for i,(filename, numpages, text) in enumerate(sorted(corpus, key=display_order)):
        print '%2d: %40s, text = %5.1f mb, numpages = %6d, %3.3f mb/page' % (i, 
            filename,  mb(len(text)), numpages, mb(len(text)/numpages))

    def sufficient(s):
        """Return True if s is repeated numpages or more times in each test file"""
        return all(text.count(s) >= numpages for (_,numpages,text) in corpus)
    
    def get_valid(base_ngrams):
        """Return list of ngrams in base_ngrams that repeated a sufficient number
            of times in the test file corpus.
        """
        return [s for s in base_ngrams if sufficient(s)]
    
    # Main loop
    #   Construct <n+1>grams from valid <n>grams until there 
    #    are no valid <n+1> grams
    #   There are at most 512 as many <n+1>grams as <n>grams so the hope
    #    is that a substantial fraction of <n>grams are invalid for low n
     
    # Naming:
    #   ngrams = valid strings of length n
    #   n1grams = valid strings of length n+1
    #   unigrams = valid strings of length 1
    
    # Shorter files first might speed up mismatch detection??
    corpus.sort(key = lambda x: len(x[2]))
    
    # Start with valid single byte strings    
    base_unigrams = [chr(i) for i in range(256)]
    unigrams = get_valid(base_unigrams)
    ngrams = unigrams
    
    while True:
        # <n+1>grams must contain valid <n>grams. There are two possible ways to
        #  construct <n+1>grams from <n>grams
        base_n1grams = set([c1 + c2 for c1 in ngrams for c2 in unigrams]) \
                     | set([c1 + c2 for c1 in unigrams for c2 in ngrams])
        # Filter down to the valid <n+1>grams
        n1grams = get_valid(base_n1grams)

        # Stop when there are no <n+1>grams or n >= 40
        if not n1grams or len(n1grams[0]) > 40:
            break

        # Repeat for n <- n+1    
        ngrams = n1grams
        
        # Report progress. Important in cases where progress is slow.
        print '-' * 40, '%d chars, %d instances' % (len(ngrams[0]), len(ngrams))
        print sorted(ngrams) 

    # ngrams now contains strings with >= numpage repeats of string
    # We need to find ngrams with exactly numpage repeats
     
    # results[s] = (filename, numpages, repeats) for ngram s
    results = {}
    for s in ngrams:
        results[s] = sorted([(filename, numpages, text.count(s)) 
            for (filename, numpages, text) in corpus], 
            key=display_order) 

    def exact_match(s): 
        """Return True if ngram s has exactly numpage repeats"""
        _, numpages, repeats = results[s]
        return numpages == repeats
 
    # Report details of all ngram matches against the corpus including non-exact
    for j,s in enumerate(sorted(results.keys(), key=lambda x:(exact_match(x),x))):
        print '-' * 80
        print '%2d: len=%d %s %s' % (j, lens(s), [s], 
            'Exact' if exact_match(x) else '')
        for i,(filename,numpages,repeats) in enumerate(results[s]):
            warning = ' MISMATCH' if repeats != numpages else ''    
            print '%2d: %40s, expected=%3d, detected=%3d %s' % (i, filename, 
                    numpages, detected, warning)

    # Finally report the ngrams that exactly matched the corpus  
    print '=' * 80
    print 'Exact matches'
    for j,s in enumerate(sorted([s for s in results.keys() if exact_match(s)])):
        print '%2d: len=%d %s' % (j, len(s), [s])                    

if __name__ == '__main__':    
    import sys

    if len(sys.argv) <= 1:
        print 'Usage: python %s <filename>' % sys.argv[0]
        exit()

    analyze(get_files(sys.argv[1]))

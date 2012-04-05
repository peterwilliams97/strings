from __future__ import division

def MB(b):
    return b/1024.0/1024.0

def H(s):
    return '[%s]' % ','.join(['0x%02x' % ord(c) for c in s])
    
def H2(a):
    return '[%s]' % ','.join(['0x%02x' % x for x in a])    

def S(a):
    return ''.join([chr(x) for x in a])    
    
import glob, os    
def get_files(file_pattern):
    """Simple glob"""
    return [p for p in glob.glob(file_pattern) if not os.path.isdir(p)]    

def findall(text, pattern):
    """Returns list of all offsets of pattern in text"""
    start = 0
    offsets = []
    while True:
        ofs = text.find(pattern, start)
        if ofs < 0:
            return offsets
        offsets.append(ofs)
        start = ofs + 1

RANGE = 1024*1024       
def compress(text, numpages):
    """Markers are likely to be near start of end of pages
        We simplistically assume pages of roughly equal size 
        and use only the   data within RANGE of multiples of 
        size of file/numbers
    """
    # Offsets of for equally sized pages
    breaks = [int(len(text) * n/numpages) for n in range(numpages)] + [len(text)]
    
    # Boundaries of RANGE bytes before and after each break
    boundaries = [[max(0,i-RANGE),min(len(text),i+RANGE)] for i in breaks]
    for i in range(1, len(boundaries)):
        boundaries[i][0] = max(boundaries[i][0], boundaries[i-1][1])
    boundaries = [(s,f) for (s,f) in boundaries if f > s]  
    
    # Just the data within the boundaries    
    return ''.join([text[s:f] for (s,f) in boundaries])


import re
RE_PAGES = re.compile(r'pages(\d+)')
def get_numpages(filename):
    """Return number of pages indicated by our file naming convention"""
    return int(RE_PAGES.search(filename).group(1))    

def get_data(filename):
   """Return filename, numpages, text for filename
        text is the compressed contents of the file
   """
   numpages = get_numpages(filename)
   text = file(filename, 'rb').read()
   text = compress(text, numpages)
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
    
    # Shorter files first might speed up mismatch detection??
    corpus.sort(key = lambda x: len(x[2]))
    
    # Show the file list
    for i,(filename, numpages, text) in enumerate(corpus):
        print '%40s, text = %5.1f mb, numpages = %6d, %3.3f mb/page' % (filename, MB(len(text)), numpages, MB(len(text)/numpages))

    def sufficient(s):
        """Return True if s is repeated numpages or more times in each test file"""
        return all([text.count(s) >= numpages for (_,numpages,text) in corpus])
    
    def get_valid(base_grams):
        """Return list of grams in base_grams that repeated a sufficient number of times
            in the test file corpus
            Also prints out some progress information
        """
        valid = [s for s in base_grams if sufficient(s)]
        if len(valid):
            print '-' * 40, '%d chars, %d instances' % (len(valid[0]), len(valid))
            print sorted(valid)
        return valid
    
    # Start with all single character strings    
    base_unigrams = [chr(i) for i in range(256)]
    unigrams = get_valid(base_unigrams)
    ngrams = unigrams
    
    while True:
        # n1grams must contain valid ngrams. There are two possible ways to
        #  construct n1grams from ngrams
        base_n1grams = set([c1 + c2 for c1 in ngrams for c2 in unigrams]) \
                     | set([c1 + c2 for c1 in unigrams for c2 in ngrams])
        # Filter down to the valid ngrams
        n1grams = get_valid(base_n1grams)

        # Stop when there are no n1grams
        if not n1grams or len(n1grams[0]) > 40:
            break
        ngrams = n1grams
 
    # Sort corpus to a nice order for viewing
    corpus.sort(key = lambda x: (x[1],len(x[2])))
    # Write out full counts
    for j,s in enumerate(ngrams):
        print '-' * 80
        print '%2d %s' % (j, [s])
        for i,(filename, numpages, text) in enumerate(corpus):
            detected = text.count(s)
            warning = ' ***' if detected != numpages else ''    
            print '%2d: %40s, expected=%3d, detected=%3d %s' % (i, filename, 
                    numpages, detected, warning)

if __name__ == '__main__':    
    import sys

    if len(sys.argv) <= 1:
        print 'Usage: python %s <filename>' % sys.argv[0]
        exit()

    analyze(get_files(sys.argv[1]))



    




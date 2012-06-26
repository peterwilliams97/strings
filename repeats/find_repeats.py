from __future__ import division

def MB(b): return b/1024.0/1024.0

def H(s):  return '[%s]' % ','.join(['0x%02x' % ord(c) for c in s])
    
def H2(a): return '[%s]' % ','.join(['0x%02x' % x for x in a])    

def S(a):  return ''.join([chr(x) for x in a])    

import re
RE_REPEATS = re.compile(r'repeats=(\d+)')
def get_numrepeats(filename):
    """Return number of repeats indicated by our file naming convention"""
    return int(RE_REPEATS.search(filename).group(1))  

def get_files(file_pattern):
    """Simple glob"""
    import glob, os    
    files = [fn for fn in glob.glob(file_pattern) if RE_REPEATS.search(fn)]   
    if not files:
        print 'No files in "%s" match pattern "%s"' % (file_pattern, RE_REPEATS.pattern)
        return None
    return files

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

def get_data(filename):
   """Return filename, numrepeats, text for filename
        text is the compressed contents of the file
   """
   numrepeats = get_numrepeats(filename)
   text = file(filename, 'rb').read()
   return filename, numrepeats, text

MAX_FILE_LEN = 999*1024*1024

def analyze(file_pattern):

    files = get_files(file_pattern)
    if not files:
        return

    corpus = [get_data(filename) for filename in files]
    corpus.sort(key = lambda x: len(x[2])/x[1])
    corpus = [x for x in corpus if len(x[2]) <= MAX_FILE_LEN]

    for i,(filename, numrepeats, text) in enumerate(corpus):
        print '%40s, text = %5.1f mb, numrepeats = %6d, %3.3f mb/repeat' % (filename, MB(len(text)), numrepeats, MB(len(text)/numrepeats))

    def sufficient(s):
        """Return True if s is repeated numrepeats or more times in each test file"""
        return all([text.count(s) >= numrepeats for (_,numrepeats,text) in corpus])

    def exact_match(s, intercept):
        """Return True if s is repeated numrepeats times in each test file"""
        return all([text.count(s) == numrepeats + intercept for (_,numrepeats,text) in corpus])    
        
    def update_exact_nstrings(nstrings, exact_nstrings):
        for i in range(20):
            exact = [s for s in nstrings if exact_match(s, i)]
            if exact:
                exact_nstrings = exact
                break    
        return exact_nstrings        
    
    def get_valid(base_strings):
        """Return list of strings in base_strings that repeated a sufficient number of times
            in the test file corpus
            Also prints out some progress information
        """
        return [s for s in base_strings if sufficient(s)]

    base_unistrings = [chr(i) for i in range(256)]
    unistrings = get_valid(base_unistrings)
    nstrings = unistrings
    exact_nstrings = None

    while True:
        # n1strings must contain valid nstrings. There are two possible ways to
        #  construct n1strings from nstrings
        base_n1strings = set([s + c for s in nstrings for c in unistrings]
                           + [c + s for s in nstrings for c in unistrings])
        base_n1strings = [w for w in words1 if w[1:] in nstrings and w[:-1] in nstrings]                   
        # Filter down to the valid nstrings
        n1strings = get_valid(base_n1strings)

        # Stop when there are no n1strings
        if not n1strings or len(n1strings[0]) > 500:
            break

        print '-' * 40, '%2d x %2d-char strings from %2d x %2d' % (len(n1strings),
                    len(n1strings[0]), len(nstrings), len(nstrings[0]))
        print n1strings            

        nstrings = n1strings
        exact_nstrings = update_exact_nstrings(nstrings, exact_nstrings)
 
    # Sort corpus to a nice order for viewing
    corpus.sort(key = lambda x: (x[1],len(x[2])))
    # Write out full counts
    for j,s in enumerate(nstrings):
        print '%2d %s %s' % (j, H(s), '-' * (60 - len(s)))
        for i,(filename, numrepeats, text) in enumerate(corpus):
            detected = text.count(s)
            warning = ' ***' if detected != numrepeats else ''    
            print '%2d: %40s, expected=%3d, detected=%3d %s' % (i, filename, 
                    numrepeats, detected, warning)

    print '=' * 80
    if exact_nstrings:
        print 'EXACT MATCHES. Length=%d. Intercept=%d' % (len(exact_nstrings[0]), i) 
        for j,s in enumerate(exact_nstrings):
            print '%2d %s %s' % (j, H(s), '-' * (60 - len(s)))
            for i,(filename, numrepeats, text) in enumerate(corpus):
                detected = text.count(s)
                warning = ' ***' if detected != numrepeats else ''    
                print '%2d: %40s, expected=%3d, detected=%3d %s' % (i, filename, 
                        numrepeats, detected, warning)    

if __name__ == '__main__':    
    import sys, time

    if len(sys.argv) <= 1:
        print 'Usage: python %s <filename>' % sys.argv[0]
        exit()

    start = time.clock()
    analyze(sys.argv[1])
    duration = time.clock() - start
    print 'duration = %.1f' % duration

# Comments, I've made a few. But then again, too few to mention.
import glob, os, re, sys
assert len(sys.argv) > 1, 'Usage: python %s <file pattern>' % sys.argv[0]
files = [fn for fn in glob.glob(sys.argv[1]) if not os.path.isdir(fn)]
corpus = [(int(re.search(r'repeats(\d+)', fn).group(1)), file(fn, 'rb').read()) 
        for fn in files]
assert corpus, r'No files matching "repeats(\d+)"'

def valid(words):
    return [w for w in words if all(text.count(w) >= n for (_,n,text) in corpus)]
    
words = chars = valid([chr(i) for i in range(256)])
while True:
    words1 = valid(set([w + c for w in words for c in chars]  
                     + [c + w for w in words for c in chars]))
    if not words1:
        break
    words = words1
print [w for w in words if all(text.count(w) == n for (_,n,text) in corpus)]    

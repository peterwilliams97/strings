# Java programmers should uncomment the following line
#from __future__ import braces

"""Find the longest substring that is repeated a specified minimum number of times a list of 
    strings. The number of repeats may different for each string.
    
    In the sample code, the strings are files with the minimum number of occcurrences of the 
    substring encoded in their names.
    
    MAIN FUNCTIONS
    --------------
    find_repeated_substrings() 
        Find longest substring that is repeated specified number of times in a list of string
    
    find_and_show_substrings()
        Reads a list of files, calls find_repeated_substrings() and prints the longest string
    
    SKETCH OF ALGORITHM
    -------------------
    for k = 4 to K
        allowed_substrings = None
        for s = shortest to longest string
            r = number of repeats required for s
            all_substrings = all substrings of length k in s that occur >= r times
            if allowed_substrings
                allowed_substrings = intersection(all_substrings, allowed_substrings)
            else    
                allowed_substrings = all_substrings
            offsets[s] = offsets of allowed_substrings in s
      
    PERFORMANCE
    -----------
    There are several aspects of the code that give good typical runtimes
        - len(allowed_substrings) cannot increase. If the first string searched is short enough 
            then len(allowed_substrings) can start at around 100-200
        - for k > 4 the length k+1 substrings are generated from the length k stings by searching 1
            character forward and back. This is 
            running_time <= 2*len(allowed_substrings)*number of strings*(K-4)*string_match(K)
    
    For typcial values of 
           starting len(allowed_substrings) 100
           number of strings 60
           K 40
    this gives       
           running_time <= 2 * 100 * 60 * 40 * 40 = 19,200,000 
"""
import sys
import glob
import re
import os

# Logging and debugging flages
_verbose = False
def set_verbose(verbose):
    global _verbose
    _verbose = verbose

_dump_dict_on = False
def set_dump_dict(dump_dict_on):
    global _dump_dict_on
    _dump_dict_on = dump_dict_on

def report(message):
    """Write <message> to stdout if _verbose is enabled"""
    if _verbose:
        print message
#  
# Some functions for displaying the offset dict structure used through this moduel
#
def offsets_string(file_names, offsets_dict):
    string = ''
    for f in file_names:
        string += f + '\n'
        for key in sorted(offsets_dict[f].keys())[:5]:
            val = sorted(list(offsets_dict[f][key]))
            string += '  ' + to_hex(key) + ':' + str(len(val)) + ': '  + str(val[:5]) + '\n'
    return string

def show_offsets_dict(file_names, offsets_dict):
    print offsets_string(file_names, offsets_dict)

if _dump_dict_on:
    DUMP_DIR = 'dumps'
    try:
        os.mkdir(DUMP_DIR)
    except:
        pass

def dump_dict(dumpfilename, file_names, offsets_dict):
    """Dump <offsets_dict> to file DUMP_DIR/<dumpfilename>"""
    if _dump_dict_on:
        file(os.path.join(DUMP_DIR, dumpfilename), 'wt').write(offsets_string(file_names, offsets_dict))

def is_junk(substring):
    """Return True if we don't want to use <substring>
        We currenly reject substring that contain nothing but space and ASCII 0s"""
    return len(substring.strip(' \t\0')) == 0

def filter_junk_strings(substrings):
    """Filter out miscellaneous junk strings"""
    filtered_substrings = {}
    for k,v in substrings.items():
        if not is_junk(k):
            filtered_substrings[k] = v
    return filtered_substrings

def get_substrings(string, k, allowed_substrings):
    """Return all substrings of length >= <k> in <string> as a dict of 
        substring:count where there are count occurrences of substring in 
        <string>
        If <allowed_substrings> then only allow substrings from <allowed_substrings>. 

        Performance:
        ------------
        The returned substring:count dict will no longer than 
        <allowed_substrings> or <parent_keys> so the best way to guarantee 
        performance is to find short key sets.
    """ 
    report('get_substrings:k=%d,allowed_substrings=%d' % (k, len(allowed_substrings) if allowed_substrings else 0))
    substrings = {}
    n = len(string)
    for i in range(n-k):
        seq = string[i:i+k]
        if is_junk(seq):
            continue
        if allowed_substrings: 
            if not seq in allowed_substrings:
                continue
        if not seq in substrings:
            substrings[seq] = 0
        substrings[seq] += 1
    return substrings

def filter_repeats(substrings, min_repeats):
    """Filter dict <substrings> for entries with values >= <min_repeats>"""
    filtered_substrings = {}
    for key,value in substrings.items():
        if value >= min_repeats:
            filtered_substrings[key] = value
    return filtered_substrings
 
def get_substring_offsets(string, substring):
    """Return set of offsets of <substring> in <string>."""
    offsets = []
    ofs = -1
    while True:
        ofs = string.find(substring, ofs+1)
        if ofs < 0:
            break
        offsets.append(ofs)
    return set(offsets)

def get_child_offsets(file_names, test_files, offsets_dict, k):
    """ Given a set of substrings of length <k> defined by offsets in a set of 
        test_files, find all substrings of length k+1
        where
            offsets_dict[<filename>][<substring>] is the set of offsets of <substring>
            in test_files[<filename>]
        <file_names> is keys of test_files in the desired sort order (shorter first)
    """
    report('get_child_offsets(%d,%d,%d,%d,k=%d)' % (len(file_names), len(test_files), 
                len(offsets_dict), len(offsets_dict.values()[0]), k))
    
    child_offsets_dict = {}
    allowed_substrings = None
    for f in file_names:
        x = test_files[f]
        child_offsets_dict[f] = {}
        for key, ofs_set in offsets_dict[f].items():
            # Use list which unlike a set can be indexed and sortet
            ofs_list = sorted(ofs_set) 
            # Remove parent offsets that would truncate substrings of length k+1
            if ofs_list[0] == 0:
                del(ofs_list[0])
            if ofs_list[-1]+k+1 == len(x['data']):
                del(ofs_list[-1]) 
            
            # Create the child k+1 length strings and add them to the child offsets dict
            for ofs in ofs_list:
                for ofs1 in [ofs-1, ofs]:
                    key1 = x['data'][ofs1:ofs1+k+1]
                    assert(len(key1) == k+1)
                    if allowed_substrings:    
                        if not key1 in allowed_substrings:
                            continue
                    if not key1 in child_offsets_dict[f].keys():
                        child_offsets_dict[f][key1] = set([])
                    child_offsets_dict[f][key1].add(ofs1)
 
        for key,val in child_offsets_dict[f].items():            
            if len(val) < x['repeats']:
                del(child_offsets_dict[f][key])
  
        allowed_substrings = child_offsets_dict[f].keys() 
        report('  allowed_substrings=%d' % len(allowed_substrings))

    # Need to go back and trim the substrings lists to allowed_substrings
    # If this results in a zero length list for any file then returns
    for f in file_names:
        for key in child_offsets_dict[f].keys():            
            if not key in allowed_substrings:
                del(child_offsets_dict[f][key])
        if len(child_offsets_dict[f]) == 0:
            return None

    dump_dict('dumpfile_%03d' % k, file_names, child_offsets_dict)

    for f in file_names:
        report('%s %d %d' % (f, len(child_offsets_dict[f]), len(offsets_dict[f])))
        assert(len(child_offsets_dict[f]) <= len(offsets_dict[f]))

    return child_offsets_dict   

min_k = 4               # Starting substring length
max_k = 2000            # Max substring length     

def find_repeated_substrings(test_files):
    """Return the longest substring(s) s that is repeated in <test_files>
        according to rule:
            For each x in test_files:
                s occurs at least x['repeats'] times in x['data']
        test_files[name] = {'data':data, 'repeats':repeats}
    """ 
    report('find_repeated_substrings(%s,%d,%d)' % (test_files.keys(), min_k, max_k))
    if not test_files:
        print 'no test files'
        return

    # Find the substrings that are repeated >= k times in files with k repeats
    # It is important to test shorter files first
    file_names = [x for x in test_files.keys()]
    file_names.sort(key = lambda x: len(test_files[x]['data']))
    report('file_names: %s' % file_names)
 
    # Start by finding all substrings of length min_k which is typically 4
    k = min_k
    allowed_substrings = None
    for name in file_names:
        x = test_files[name]
        substrings = get_substrings(x['data'], k, allowed_substrings)
        substrings = filter_repeats(substrings, x['repeats'])
        substrings = filter_junk_strings(substrings)
        if not substrings:
            print 'No %d character string works!' % k
            return None 
        allowed_substrings = substrings.keys() 
    report('k=%d:\n\substrings=%d:%s' % (k, len(allowed_substrings), sorted(allowed_substrings)))

    # From now on work with offsets  
    # offsets_dict[<filename>][<substring>] = list of offsets of <substring> in file with name <filename>
    offsets_dict = {}
    for name in file_names:
        x = test_files[name]
        offsets_dict[name] = {}
        for key in substrings.keys():
            offsets_dict[name][key] = get_substring_offsets(x['data'], key)

    # Work in increasing length of substrings, +1 per round    
    for k in range(min_k, max_k):
        child_offsets_dict = get_child_offsets(file_names, test_files, offsets_dict, k)
        if not child_offsets_dict:
            break
        offsets_dict = child_offsets_dict 
    
    # return last non-empty dict of substrings    
    return offsets_dict

def to_hex(s):
    """Convert string <s> to hex"""    
    return ','.join(['0x%02x' % ord(x) for x in s])

def name_to_repeats(name):
    """Parse number of repeats from <name>"""
    m = re.search(r'_pages(\d+)_', name)
    return int(m.group(1)) if m else None

def get_test_files(mask):
    """Return a list of dicts for for file that match <mask>
        Dict entries are 
        test_files[name] = {'data':data, 'repeats':repeats}
            name: file name
            page: number of repeats in file
            data: file contents as string
    """
    test_files = {}
    for name in glob.glob(mask):
        if not os.path.isdir(name):
            repeats = name_to_repeats(name)
            if repeats:
                data = file(name, 'rb').read()
                if data:
                    test_files[name] = {'repeats':repeats, 'data':data}
    return test_files
    
def find_and_show_substrings(mask):
    """Find the longest substring that is repeated in a list of files
        matched by <mask> in which the filename encodes the number of 
        repeats as defined in name_to_repeats() above.
    """    

    # Read the files matched by <mask>    
    test_files = get_test_files(mask)
    print '%d files in mask "%s"' % (len(test_files), mask)
    if not test_files:
        print 'no test files'
        return

    for i,(name,x) in enumerate(sorted(test_files.items())):
        print '%2d: size=%6d, repeats=%2d, name="%s"' % (i, len(x['data']), x['repeats'], name)
    print '-' * 60

    offsets_dict = find_repeated_substrings(test_files)

    # Print out the results 
    file_names = [x for x in test_files.keys()]
    file_names.sort(key = lambda x: len(test_files[x]['data']))  
    substring_list = offsets_dict[file_names[0]].keys()
        
    print 'Substrings summary:'
    print 'Found %d substrings' % len(substring_list)
    for i, substring in enumerate(substring_list):
        print '%2d: len=%3d, substring="%s"' % (i, len(substring), substring)

    print '=' * 80
    print 'Substrings in detail:'
    for substring in substring_list:
        print 'Substring %2d' % i, '-' * 60
        print 'len=%3d, substring="%s"' % (len(substring), substring)
        print 'hex =', to_hex(substring)
        print 'Offsets of substring in test files:'
        for name in file_names:
            x = test_files[name]
            offsets = sorted(list(offsets_dict[name][substring]))
            print '%40s: needed=%-2d,num=%-2d,offsets=%s' % (name, x['repeats'], len(offsets), offsets)

if __name__ == '__main__':
    if len(sys.argv) <= 1:
        print 'Usage: python', sys.argv[0], '<file mask>'
        exit()
    _verbose = False
    find_and_show_substrings(sys.argv[1])

    
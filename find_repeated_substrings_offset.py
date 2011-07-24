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

def to_hex(s):
    """Convert string <s> to hex"""    
    return ','.join(['0x%02x' % ord(x) for x in s])

def H(s):
    h = 'H_' + ''.join(['%02x' % ord(x) for x in s])
    assert(2*len(s)+2 == len(h))
    return h

def unH(s):
    assert(len(s) % 2 == 0)
    chars = [chr(int(s[i:i+2],16)) for i in range(2, len(s),2)]
    #print len(s), s
    #print len(chars), chars
    assert(2*len(chars)+2 == len(s))
    return ''.join(chars)
    
TEST_H_STRING = 'H_000000000400'    
TEST_STRING = unH(TEST_H_STRING)
assert(H(TEST_STRING) == TEST_H_STRING)
    
# Logging and debugging flages
_verbose = False
def set_verbose(verbose):
    global _verbose
    _verbose = verbose

_dump_dict_on = False
def set_dump(dump):
    global _dump_dict_on
    _dump_dict_on = dump

_validate = False
def set_validate(validate):
    global _validate
    _validate = validate

def report(message):
    """Write <message> to stdout if _verbose is enabled"""
    if _verbose:
        print message
#  
# Some functions for displaying the offset dict structure used through this moduel
#
def _od_substrings(offsets_dict):
    """Return substrings in <offsets_dict>. This is the same for each file."""
    substrings = sorted(offsets_dict[offsets_dict.keys()[0]].keys())
    if True:
        for name in offsets_dict:
            subs = sorted(offsets_dict[name].keys())
            if subs != substrings:
                print name
                print '-' * 60
                print '      subs =', subs
                print '-' * 60
                print 'substrings =', substrings
                print '-' * 60
            assert(subs == substrings)
    return substrings

def _od_substrings_string(offsets_dict):
    """Return string of list of substrings in <offsets_dict>"""
    return '\n'.join([H(s) for s in _od_substrings(offsets_dict)])

def _od_offsets_string(file_names, offsets_dict):
    """Return string representation of <offsets_dict> sorted by <file_names>"""
    string = ''
    for f in file_names:
        string += f + '\n'
        for key in sorted(offsets_dict[f].keys())[:5]:
            val = sorted(list(offsets_dict[f][key]))
            string += '  %s:%3d:%s\n' % (H(key), len(val), val[:5])
    return string

SEPARATOR = ','
def _od_offsets_matrix(file_names, offsets_dict, test_files):
    """Return matrix representation of <offsets_dict> sorted by <file_names>"""
    string = ''
    string += SEPARATOR.join(['name'] + file_names) + '\n'
    string += SEPARATOR.join(['repeats'] + ['%d' % test_files[f]['repeats'] for f in file_names]) + '\n'
    string += SEPARATOR.join(['size'] + ['%d' % len(test_files[f]['data']) for f in file_names]) + '\n'
    for subs in _od_substrings(offsets_dict):
        string += SEPARATOR.join([H(subs)] + ['%d' % len(offsets_dict[f][subs]) for f in file_names]) + '\n'
    return string

def show_offsets_dict(file_names, offsets_dict):
    print _od_offsets_string(file_names, offsets_dict)

DUMP_DIR = 'dumps'
try:
    os.mkdir(DUMP_DIR)
except:
    pass
def dump_dict(dumpfilename, file_names, test_files, offsets_dict):
    """Dump <offsets_dict> to file DUMP_DIR/<dumpfilename>"""
    if _dump_dict_on:
        file(os.path.join(DUMP_DIR, dumpfilename + '.txt'), 'wt').write(_od_offsets_string(file_names, offsets_dict))
        file(os.path.join(DUMP_DIR, dumpfilename + '.strings.txt'), 'wt').write(_od_substrings_string(offsets_dict))
        matrix = _od_offsets_matrix(file_names, offsets_dict, test_files)
        file(os.path.join(DUMP_DIR, dumpfilename + '.csv'), 'wt').write(matrix)

# String finding code starts here!  

def is_junk(substring):
    """Return True if we don't want to use <substring>
        We currenly reject substring that contain nothing but space and ASCII 0s"""
    return False  # !@#$ Need to fix this!
    return len(substring.strip(' \t\0')) == 0

def contains_junk(substring):
    for i in range(len(substring)-min_k):
        if is_junk(substring[i:i+min_k]):
            return True
    return False
    
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

        Performance
        -----------
        The returned substring:count dict will no longer than 
        <allowed_substrings> or <parent_keys> so the best way to guarantee 
        performance is to find short key sets.
    """ 
    report('get_substrings:k=%2d,allowed_substrings=%3d,size=%7d' % 
        (k, len(allowed_substrings) if allowed_substrings else 0, len(string)))
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

def get_matching_offsets(file_names, offsets_dict, substring):
    """Return members of <offsets_dict> that match <substring>"""
    matching_offsets = {}
    for name in file_names:
        substrings_list = sorted(offsets_dict[name].keys())
        if substring in substrings_list:
            if not name in matching_offsets.keys():
                matching_offsets[name] = {}
            matching_offsets[name] = offsets_dict[name][substring]
    return matching_offsets

def is_offsets_greater(file_names, matching_offsets1, matching_offsets2):
    """Return True if matching_offsets1 has as many entries for each name as matching_offsets2"""
    for name in file_names:
        assert(len(matching_offsets1) >= len(matching_offsets1))
        if len(matching_offsets1) < len(matching_offsets1):
            return False
    return True

def validate_child_offsets(file_names, offsets_dict, child_offsets_dict, k):
    """Check that a length k+1 substring is repeated no more than the length k substrings it 
        contains"""
    substrings_list = _od_substrings(offsets_dict) 
    child_substrings_list = _od_substrings(child_offsets_dict) 
    report('validate_child_offsets(%d)' % k) 

    for child_substring in child_substrings_list:
        assert(len(child_substring) == k+1)
        child_matching_offsets = get_matching_offsets(file_names, child_offsets_dict, child_substring)
        substring1 = child_substring[:-1]
        substring2 = child_substring[1:]
        if not substring1 in substrings_list:
            print 'No match for parent"%s", child="%s" in parent list' % (H(substring1), H(child_substring))
            return False
        if not substring2 in substrings_list:
            print 'No match for parent"%s", child="%s" in parent list' % (H(substring2), H(child_substring))
            return False
        matching_offsets1 = get_matching_offsets(file_names, offsets_dict, substring1)
        matching_offsets2 = get_matching_offsets(file_names, offsets_dict, substring1)
        if not is_offsets_greater(file_names, matching_offsets1, child_matching_offsets):
            print 'Mismatch on parent="%s", child="%s"' % (H(substring1), H(child_substring))
            return False
        if not is_offsets_greater(file_names, matching_offsets2, child_matching_offsets):
            print 'Mismatch on parent="%s", child="%s"' % (H(substring2), H(child_substring))
            return False
    return True

def get_child_offsets(file_names, test_files, offsets_dict, k):
    """ Given a set of substrings of length <k> defined by offsets in a set of 
        test_files, return a dict of substrings of length k+1
        where
            offsets_dict[<filename>][<substring>] is the set of offsets of <substring>
            in test_files[<filename>]
        <file_names> is keys of test_files in the desired sort order (shorter first)
        
        Performance
        -----------
        This is the inner loop of the program.
        The returned dict will no longer than offsets_dict and string searches are on existing
        substrings + 1 character to left or right so there is not that much text to search.
    """
    report('get_child_offsets(file_names=%d,test_files=%d,%d,substrings=%d,k=%d)' % 
        (len(file_names), len(test_files), len(offsets_dict), len(offsets_dict.values()[0]), k))
    
    child_offsets_dict = {}
    allowed_substrings = None

    for f in file_names:
        x = test_files[f]
        child_offsets_dict[f] = {}
        for key, ofs_set in offsets_dict[f].items():
            # Use list which unlike a set can be indexed and sorted
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
                    # Filter out keys with junk parents, which includes junk keys for current is_junk()
                    if is_junk(key1[1:]) or is_junk(key1[:-1]) or is_junk(key1):
                       continue
                    if contains_junk(key1):
                        continue
                        
                    if False:
                        if key1 == BOGUS_STRING or H(key1) == 'H_0000000004' or H(key1) == 'H_000000000400':
                            print f
                            print H(key1), ofs1, len(key1), k+1
                            print H(key1[1:]), is_junk(key1[1:])
                            print H(key1[:-1]), is_junk(key1[:-1])
                            for j in range(ofs1,ofs1+k+1):
                                print '  %2d:%02x' % (j, ord(x['data'][j])) 
                            exit()
                    if not key1 in child_offsets_dict[f].keys():
                        child_offsets_dict[f][key1] = set([])
                    child_offsets_dict[f][key1].add(ofs1)
 
        unpruned_len = len(child_offsets_dict[f].keys())
        for key, ofs_set in child_offsets_dict[f].items():            
            if len(ofs_set) < x['repeats']:
                del(child_offsets_dict[f][key])
  
        allowed_substrings = child_offsets_dict[f].keys() 
        report('  allowed_substrings=%3d,%3d,size=%7d' % 
            (unpruned_len, len(allowed_substrings), len(x['data'])))

    # Need to go back and trim the substrings lists to allowed_substrings
    # If this results in a zero length list for any file then returns
    for f in file_names:
        for key in child_offsets_dict[f].keys():            
            if not key in allowed_substrings:
                del(child_offsets_dict[f][key])
        if len(child_offsets_dict[f]) == 0:
            return None

    dump_dict('dumpfile_%03d' % (k+1), file_names, test_files, child_offsets_dict)
    
    if _validate:
        if not validate_child_offsets(file_names, offsets_dict, child_offsets_dict, k):
            raise ValueError

    for f in file_names:
        report('before=%3d,after=%3d,file=%s' % (len(offsets_dict[f]),len(child_offsets_dict[f]),f))
        # !@#$ assert is needed!!
        #assert(len(child_offsets_dict[f]) <= len(offsets_dict[f]))

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

    report('file_names:\n%s' % '\n'.join(['%8d:%3d: %s' % 
            (len(test_files[f]['data']),test_files[f]['repeats'],f) for f in file_names]))

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

    # Remove all the substrings that are no longer used
    for name in file_names:
        for key in substrings.keys():
            if not key in allowed_substrings:
                del(substrings[name][key])
    report('k=%d:\n\substrings=%d:%s' % (k, len(allowed_substrings), sorted(allowed_substrings)))

    # From now on work with offsets  
    # offsets_dict[<filename>][<substring>] = list of offsets of <substring> in file with name <filename>
    offsets_dict = {}
    for name in file_names:
        x = test_files[name]
        offsets_dict[name] = {}
        for key in substrings.keys():
            offsets_dict[name][key] = get_substring_offsets(x['data'], key)

    dump_dict('dumpfile_%03d' % min_k, file_names, test_files, offsets_dict)
    # Work in increasing length of substrings, +1 per round    
    for k in range(min_k, max_k):
        child_offsets_dict = get_child_offsets(file_names, test_files, offsets_dict, k)
        if not child_offsets_dict:
            break
        offsets_dict = child_offsets_dict 
    
    # return last non-empty dict of substrings    
    return offsets_dict

# String finding code ends here!  

# Display code starts here

def name_to_repeats(name):
    """Parse number of repeats from <name>"""
    m = re.search(r'(?:\s|-)pages=(\d+)', name)
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
        print '%2d: size=%7d, repeats=%3d, name="%s"' % (i, len(x['data']), x['repeats'], name)
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
    from optparse import OptionParser
    parser = OptionParser()
    parser.add_option('-v', '--verbose', action="store_true", dest='verbose', default=False,
                help='print status messages to stdout')
    parser.add_option('-d', '--dump', action="store_true", dest='dump', default=False,
                help='dump data to files')
    parser.add_option('-a', '--validate', action="store_true", dest='validate', default=False,
                help='perform validation of intermediate results')           
    (options, args) = parser.parse_args()           

    if len(args) < 1:
        print 'Usage: python', sys.argv[0], '<file mask>'
        print '--help for more information'
        exit()

    set_verbose(options.verbose)
    set_dump(options.dump)
    set_validate(options.validate)
    print 'options =', options

    find_and_show_substrings(args[0])


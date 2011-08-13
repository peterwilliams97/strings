"""Find the longest substring that is repeated a specified minimum number of times a list of 
    strings. The number of repeats may different for each string.
    
    In the sample code, the strings are files with the minimum number of occcurrences of the 
    substring encoded in their names.
    
    MAIN FUNCTIONS
    --------------
    find_repeated_substrings() 
        Find longest substring that is repeated specified number of times in a list of strings
    
    find_and_show_substrings()
        Reads a list of files, calls find_repeated_substrings() and prints the longest substring

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
        - len(allowed_substrings) cannot increase for a given k and tends not to grow very much for 
            as k increases. If the first string searched is short enough then len(allowed_substrings) 
            can start at around 100-200 and stay below 300.
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
import time
import random
import common
from common import H, unH
import rolling_hash

def filter_junk_strings(substrings):
    """Filter out miscellaneous junk strings"""
    filtered_substrings = {}
    for k,v in substrings.items():
        if not common.is_junk(k):
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
    common.report('get_substrings:k=%2d,allowed_substrings=%5d,size=%7d' % 
        (k, len(allowed_substrings) if allowed_substrings else -1, len(string)))
    substrings = {}
    n = len(string)
    for i in range(n-k):
        pattern = string[i:i+k]
        if common.is_junk(pattern):
            continue
        if allowed_substrings: 
            if not pattern in allowed_substrings:
                continue
        if not pattern in substrings:
            substrings[pattern] = 0
        substrings[pattern] += 1
    return substrings

def filter_repeats(substrings, min_repeats):
    """Return entries in dict <substrings> with values >= <min_repeats>"""
    filtered_substrings = {}
    for key,value in substrings.items():
        if value >= min_repeats:
            filtered_substrings[key] = value
    return filtered_substrings

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
    common.report('validate_child_offsets(%d)' % k) 

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
    common.report('get_child_offsets(file_names=%d,test_files=%d,%d,substrings=%d,k=%d)' % 
        (len(file_names), len(test_files), len(offsets_dict), len(offsets_dict.values()[0]), k))
    
    parent_substrings = offsets_dict[file_names[0]].keys()
    child_offsets_dict = {}
    allowed_substrings = None

    for name in file_names:
        x = test_files[name]
        child_offsets_dict[name] = {}
        
        for key, ofs_set in offsets_dict[name].items():
            # Use a list which unlike a set can be indexed and sorted
            ofs_list = sorted(ofs_set) 
            # Remove parent offsets that would truncate substrings of length k+1
            if ofs_list[0] == 0:
                del(ofs_list[0])
            if ofs_list[-1]+k+1 == len(x['text']):
                del(ofs_list[-1]) 

            # Create the child length k+1 substrings and add them to the child offsets dict
            # ofs1 is the offset of the k+1 substring key1 
            for ofs in ofs_list:
                for ofs1 in [ofs-1, ofs]:
                    key1 = x['text'][ofs1:ofs1+k+1]
                    assert(len(key1) == k+1)
                    if allowed_substrings:    
                        if not key1 in allowed_substrings:
                            continue
                    # Only allow keys with valid parents
                    if not key1[1:] in parent_substrings or not key1[:-1] in parent_substrings:
                        continue
                    # Get rid of the junk too    
                    if common.is_junk(key1):
                        continue

                    # Got through all the filters. Add the new offset to the child dict
                    if not key1 in child_offsets_dict[name].keys():
                        child_offsets_dict[name][key1] = set([])
                    child_offsets_dict[name][key1].add(ofs1)
 
        # Prume the entries with insufficient repeats
        unpruned_len = len(child_offsets_dict[name].keys())
        for key, ofs_set in child_offsets_dict[name].items():            
            if len(ofs_set) < x['repeats']:
                del(child_offsets_dict[name][key])
  
        # allowed_substrings is used as a filter in all but first pass through this loop
        allowed_substrings = child_offsets_dict[name].keys() 
        common.report('  allowed_substrings=%3d,%3d,size=%7d' % 
            (unpruned_len, len(allowed_substrings), len(x['text'])))

    # Need to go back and trim the substrings lists to allowed_substrings
    # If this results in a zero length list for any file then returns
    for name in file_names:
        for key in child_offsets_dict[name].keys():            
            if not key in allowed_substrings:
                del(child_offsets_dict[name][key])
        if len(child_offsets_dict[name]) == 0:
            return None

    common.dump_dict('dumpfile_%03d' % (k+1), file_names, test_files, child_offsets_dict)
    
    if common.is_validate():
        if not validate_child_offsets(file_names, offsets_dict, child_offsets_dict, k):
            raise ValueError

    for name in file_names:
        common.report('before=%3d,after=%3d,file=%s' % (len(offsets_dict[name]),
            len(child_offsets_dict[name]),name))

    return child_offsets_dict   
    
def get_offsets_from_texts(file_names, test_files, k):
    common.note_time('get_offsets_from_texts k=%d' % k)
    
    allowed_substrings = None
    for name in file_names:
        x = test_files[name]
        substrings = get_substrings(x['text'], k, allowed_substrings)
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
    #report('k=%d:\n\substrings=%d:%s' % (k, len(allowed_substrings), sorted(allowed_substrings)))
    note_time('got substrings')

    # From now on work with offsets  
    # offsets_dict[<filename>][<substring>] = list of offsets of <substring> in file with name <filename>
    offsets_dict = {}
    for name in file_names:
        x = test_files[name]
        offsets_dict[name] = {}
        for key in substrings.keys():
            offsets_dict[name][key] = common.get_substring_offsets(x['text'], key) 

    return [offsets_dict[name] for name in file_names] 

def test_files_to_text_repeats(file_names, test_files):
    text_list = [test_files[name]['text'] for name in file_names]
    min_repeats_list = [test_files[name]['repeats'] for name in file_names]
    return text_list, min_repeats_list

_MIN_K = 4              # Starting substring length
_MAX_K = 2000           # Max substring length     

def find_repeated_substrings(test_files):
    """Return the longest substring(s) s that is repeated in <test_files>
        according to rule:
            For each x in test_files:
                s occurs at least x['repeats'] times in x['text']
        test_files[name] = {'text':text, 'repeats':repeats}
    """ 
    common.note_time('start searching strings')
    common.report('find_repeated_substrings(%d,%d,%d)' % (len(test_files.keys()), _MIN_K, _MAX_K))
    if not test_files:
        print 'no test files'
        return

    # Find the substrings that are repeated >= k times in files with k repeats
    # It is important to test shorter files first
    file_names = [x for x in test_files.keys()]
    file_names.sort(key = lambda x: len(test_files[x]['text']))

    common.report('file_names:\n%s' % '\n'.join(['%8d:%3d: %s' % 
            (len(test_files[name]['text']),test_files[name]['repeats'],name) for name in file_names]))

    # Start by finding all substrings of length _MIN_K which is typically 4
    k = _MIN_K
    
    if False:
        print 'Pure Python'
        pattern_offsets_list = get_offsets_from_texts(file_names, test_files, k)
    else:
        print 'Cython rolling hash'
        text_list, repeats_list = test_files_to_text_repeats(file_names, test_files)
        pattern_offsets_list = rolling_hash.get_offsets_from_texts(text_list, repeats_list, k)    
    if False:
        # Does not work. !@#$ Find out why
        while k >= _MIN_K:  
            pattern_offsets_list = rolling_hash.get_offsets_from_texts(text_list, repeats_list, k)
            if pattern_offsets_list[0]:
                break
            print 'reducing k %d=>%d' % (k, k // 2)
            k = k // 2
            
    common.note_time('got substrings')

    offsets_dict = dict(zip(file_names, pattern_offsets_list))
    # Work in increasing length of substrings, +1 per round    
    for k in range(_MIN_K, _MAX_K):
        common.note_time('found %3d substrings of length >= %3d' % (len(offsets_dict[file_names[0]]), k)) 
        child_offsets_dict = get_child_offsets(file_names, test_files, offsets_dict, k)
        if not child_offsets_dict:
            break
        offsets_dict = child_offsets_dict 

    # return last non-empty dict of offsets    
    return offsets_dict

    # return last non-empty dict of offsets
    offsets_dict = dict(zip(file_names, pattern_offsets_list))
    return offsets_dict

 
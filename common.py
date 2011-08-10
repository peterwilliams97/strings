"""
Some shared routines
"""
import sys
import glob
import re
import os
import time
import random
import pickle

def H(s):
    """Returns a readable and invertab;e hex reprenstation of <s> as a string"""
    return 'H_' + ''.join(['%02x' % ord(x) for x in s])

def unH(s):
    """Inverse of H(). y = H(x) <=> x = unH(y)"""
    return ''.join([chr(int(s[i:i+2],16)) for i in range(2, len(s),2)]) 

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

_DUMP_DIR = 'dumps'
try:
    os.mkdir(_DUMP_DIR)
except:
    pass

def _dumpify(filename):
    return os.path.join(_DUMP_DIR, filename)

_start_time = time.time()
_time_file = open(_dumpify('timing.txt'), 'wt')

def note_time(desc):
    if _quiet:
        return
    global _time_file
    msg = '%4.1f sec: %s' % (time.time() - _start_time, desc)
    print 'time> %s' % msg
    _time_file.write(msg + '\n')

# Logging and debugging flags
_quiet = False
def set_quiet(quiet):
    global _quiet
    _quiet = quiet
    if _quiet:
        _verbose = False

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
    
def is_validate():
    return _validate

def report(message):
    """Write <message> to stdout if _verbose is enabled"""
    if _verbose:
        print message
#  
# Some functions for displaying the offset dict structure used through this moduel
#
def _od_substrings(offsets_dict):
    """Return substrings in <offsets_dict>. This is the same for each file."""
    return sorted(offsets_dict[offsets_dict.keys()[0]].keys())

def _od_substrings_string(offsets_dict):
    """Return string of list of substrings in <offsets_dict>"""
    return '\n'.join([H(s) for s in _od_substrings(offsets_dict)])

def _od_offsets_string(file_names, offsets_dict):
    """Return string representation of <offsets_dict> sorted by <file_names>"""
    string = ''
    for name in file_names:
        string += name + '\n'
        for key in sorted(offsets_dict[name].keys())[:5]:
            val = sorted(list(offsets_dict[name][key]))
            string += '  %s:%3d:%s\n' % (H(key), len(val), val[:5])
    return string

SEPARATOR = ','
def _od_offsets_matrix(file_names, offsets_dict, test_files):
    """Return matrix representation of <offsets_dict> sorted by <file_names>"""
    string = ''
    string += SEPARATOR.join(['name'] + file_names) + '\n'
    string += SEPARATOR.join(['repeats'] + ['%d' % test_files[name]['repeats'] for name in file_names]) + '\n'
    string += SEPARATOR.join(['size'] + ['%d' % len(test_files[name]['data']) for name in file_names]) + '\n'
    for subs in _od_substrings(offsets_dict):
        string += SEPARATOR.join([H(subs)] + ['%d' % len(offsets_dict[name][subs]) for name in file_names]) + '\n'
    return string

def show_offsets_dict(file_names, offsets_dict):
    print _od_offsets_string(file_names, offsets_dict)

def dump_dict(dumpfilename, file_names, test_files, offsets_dict):
    """Dump <offsets_dict> to file DUMP_DIR/<dumpfilename>"""
    if _dump_dict_on:
        file(_dumpify(umpfilename + '.txt'), 'wt').write(_od_offsets_string(file_names, offsets_dict))
        file(o_dumpify('.strings.txt'), 'wt').write(_od_substrings_string(offsets_dict))
        matrix = _od_offsets_matrix(file_names, offsets_dict, test_files)
        file(_dumpify(dumpfilename + '.csv'), 'wt').write(matrix)

_P1 = 153101159        
_P2 = 982451653

def _compress_name(name):
    """Return <name> compressed to 9 characters"""
    n = 0
    for c in name:
        x = ord(c)
        n = (n * _P1 + x) % _P2 
    return '%09d' % n

def save_to_disk(name, object):
    """Pickle <object> and save it in file  named with hash of <name>"""
    shortname = _dumpify(_compress_name(name) + '.pkl')
    print 'save_to_disk(%s)' % shortname
    pkl_file = open(shortname , 'wb')
    pickle.dump(object, pkl_file, -1)   # Pickle the list using the highest protocol available.
    pkl_file.close()

def load_from_disk(name):
    """Restore pickle object in file named with hash of <name>"""
    shortname = _dumpify(_compress_name(name) + '.pkl')
    print 'load_from_disk(%s)' % shortname
    pkl_file = open(shortname, 'rb')
    object = pickle.load(pkl_file)
    pkl_file.close()
    return object

if __name__ == '__main__':
    object = 'A string'
    pickle_name = 'A pickle'
    save_to_disk(pickle_name, object)
    object2 = load_from_disk(pickle_name)
    assert(object2 == object)
    
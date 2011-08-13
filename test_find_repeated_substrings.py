import glob
import os
import random
import re
import sys
import common
from common import H, unH
#import find_repeated_substrings_offset as FRS
import find_repeated_substrings_rolling_hash as FRS

def name_to_repeats(name):
    """Parse number of repeats from <name>"""
    m = re.search(r'(?:\s|-)pages=(\d+)', name)
    return int(m.group(1)) if m else None

def get_test_files(mask):
    """Return a list of dicts for for file that match <mask>
        Dict entries are 
        test_files[name] = {'text':text, 'repeats':repeats}
            name: file name
            page: number of repeats in file
            text: file contents as string
    """
    test_files = {}
    for name in glob.glob(mask):
        if not os.path.isdir(name):
            repeats = name_to_repeats(name)
            if repeats:
                text = file(name, 'rb').read()
                if text:
                    test_files[name] = {'repeats':repeats, 'text':text}
    return test_files
    
def find_and_show_substrings(mask):
    """Find the longest substring that is repeated in a list of files
        matched by <mask> in which the filename encodes the number of 
        repeats as defined in name_to_repeats() above.
    """

    # Read the files matched by <mask>    
    test_files = get_test_files(mask)
    file_names = [x for x in test_files.keys()]
    file_names.sort(key = lambda x: len(test_files[x]['text']))  
    
    print '%d files in mask "%s"' % (len(test_files), mask)
    if not test_files:
        print 'no test files'
        return

    for i, name in enumerate(file_names):
        x = test_files[name]
        print '%2d: size=%7d, repeats=%3d, name="%s"' % (i, len(x['text']), x['repeats'], name)
    print '-' * 60

    offsets_dict = FRS.find_repeated_substrings(test_files)

    # Print out the results 
    
    substring_list = offsets_dict[file_names[0]].keys()

    print 'Substrings summary:'
    print 'Found %d substrings' % len(substring_list)
    for i, substring in enumerate(substring_list):
        print '%2d: len=%3d, substring="%s"' % (i, len(substring), H(substring))

    print '=' * 80
    print 'Substrings in detail:'
    for substring in substring_list:
        print 'Substring %2d' % i, '-' * 60
        print 'len=%3d, substring="%s"' % (len(substring), substring)
        print 'hex =', H(substring)
        print 'Offsets of substring in test files:'
        for name in file_names:
            x = test_files[name]
            offsets = sorted(offsets_dict[name][substring])
            print '%40s: needed=%-2d,num=%-2d,offsets=%s' % (os.path.basename(name), 
                x['repeats'], len(offsets), offsets)
            offsets2 = sorted(common.get_substring_offsets(x['text'], substring)) 
            assert(len(offsets2) == len(offsets))
            for i in range(len(offsets)):
                assert(offsets2[i] == offsets[i])

def compare_string_subsets(mask, subset_fraction, num_tests):
    """Find the longest substring that is repeated in several subsets of a list of files
        matched by <mask> in which the filename encodes the number of repeats as defined 
        in name_to_repeats() above.
        Compare results from each set. It shoulbe the same
    """
    # Read the files matched by <mask>    
    test_files = get_test_files(mask)
    print '%d files in mask "%s"' % (len(test_files), mask)
    if not test_files:
        print 'no test files'
        return

    file_names = [x for x in test_files.keys()]
    file_names.sort(key = lambda x: len(test_files[x]['text']))  
    
    for i, name in enumerate(file_names):
        x = test_files[name]
        print '%2d: size=%7d, repeats=%3d, name="%s"' % (i, len(x['text']), x['repeats'], name)
    print '=' * 60

    subset_size = int(len(test_files)*subset_fraction)

    print 'Testing %d subsets of size %d from total of %d' % (num_tests, subset_size, len(test_files))

    random.seed(111)

    subset_substring_list_list = [] 
    for test in range(num_tests):
        test_file_names = file_names[:]
        random.shuffle(test_file_names)
        test_files_subset = test_file_names[:subset_size]
        if not common.is_quiet():
            for i, name in enumerate(test_files_subset):
                x = test_files[name]
                print '%2d: size=%7d, repeats=%3d, name="%s"' % (i, len(x['text']), x['repeats'], name)
            print '-' * 60
        offsets_dict = FRS.find_repeated_substrings(test_files)
        substring_list = offsets_dict[file_names[0]].keys()
        subset_substring_list_list.append(substring_list)
        print 'Found %d substrings' % len(substring_list)
        for i, substring in enumerate(substring_list):
            print '%2d: len=%3d, substring="%s"' % (i, len(substring), H(substring))
            for name in file_names:
                x = test_files[name]
                offsets = sorted(offsets_dict[name][substring])
                print '%40s: needed=%-2d,num=%-2d,offsets=%s' % (os.path.basename(name), 
                    x['repeats'], len(offsets), offsets)
                offsets2 = sorted(common.get_substring_offsets(x['text'], substring)) 
                assert(len(offsets2) == len(offsets))
                for i in range(len(offsets)):
                    assert(offsets2[i] == offsets[i]) 

if __name__ == '__main__':
    from optparse import OptionParser
    parser = OptionParser()
    parser.add_option('-v', '--verbose', action="store_true", dest='verbose', default=False,
                help='print status messages to stdout')
    parser.add_option('-q', '--quiet', action="store_true", dest='quiet', default=False,
                help='print only results to stdout')
    parser.add_option('-d', '--dump', action="store_true", dest='dump', default=False,
                help='dump text to files')
    parser.add_option('-a', '--validate', action="store_true", dest='validate', default=False,
                help='perform validation of intermediate results')
    parser.add_option('-t', '--test', action="store_true", dest='test_subsets', default=False,
                help='test on subsets of texts')
    (options, args) = parser.parse_args()           

    if len(args) < 1:
        print 'Usage: python', sys.argv[0], '<file mask>'
        print '--help for more information'
        exit()

    common.set_verbose(options.verbose)
    common.set_quiet(options.quiet)
    common.set_dump(options.dump)
    common.set_validate(options.validate)
    print 'options =', options

    if options.test_subsets:
        compare_string_subsets(args[0], 0.5, 5)
    else:
        find_and_show_substrings(args[0])


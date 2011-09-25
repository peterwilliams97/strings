# -*- coding: utf-8 -*-
from __future__ import division
"""
    Unicode (binary) generalized suffix tree
    
    Goal: 
        Find longest patterns(s) that occur M[k] times in text[k] for k = 0..K-1
    
    Notes:
        Let N = sum(len(text[k])) over k = 0..K-1
        
    Method:
        1. Create generalized suffix tree for texts. O(N) time and space 
        2. Annotate each node with number of occurences in text[k]. 
            Requires bottom-up traversal of suffix tree O(N) with O(K) annotations/node.
            Total O(N*K)
        3. Traverse suffix tree to find nodes satisfing M[k] occurrences for for k = 0..K-1 and
            take longest
        
    
    Unicode is being used as a simple way of coding binary data while allowing unique termination 
    characters. 
        - Chars 0-255 are bytes from some binary lump
        - Chars >255 are termination characters
    
    This allows encoding of binary data for a doubling of string storage size which seems like a
    good trade-off that saves me trying to figure how to reprensent string endings in the various
    suffix tree classes
    
    I have lost the name of the website I copied the original version of this from. Will add it as
    soon as I find it.
    
    It will become a   
        Suffix Tree implementation based on http://marknelson.us/1996/08/01/suffix-trees/ 
"""

_POSITIVE_INFINITY = 1 << 30

def _infi(n):
    if n == _POSITIVE_INFINITY:
        return ''
    return str(n)
    
def _uc(s):
    return ','.join(['%02x'%ord(c) for c in s])
    return s.decode('utf8', errors='replace')
    
def _q(s):
    return '<' + _uc(s) + '>'    

class Node:
    def __init__(self, suffix_link=None):
        self.suffix_link = suffix_link

    def __repr__(self):
        return 'Node(%s)' % self.suffix_link

class Edge:
    """Links 2 nodes in a suffix tree
        src_node_idx, dst_node_idx are nodes that edge links
        string_idx, first_char_idx, last_char_idx specify the string segment that edge encodes 
    """
    def __init__(self, suffix_tree, src_node_idx, dst_node_idx, string_idx, first_char_idx, last_char_idx):
        assert(string_idx >= 0)
        self.suffix_tree = suffix_tree # for debugging
        self.src_node_idx = src_node_idx
        self.dst_node_idx = dst_node_idx
        self.string_idx = string_idx
        self.first_char_idx = first_char_idx
        self.last_char_idx = last_char_idx
        print self

    def split(self, suffix, suffix_tree, string_idx):
        #print '  split(%s,%s)' % (self, suffix)
        return split_edge(self, suffix, suffix_tree, string_idx)
    
    def __len__(self):
        return len(self.get_string()) # Inefficient !@#$
        
    def get_string(self):
        #print 'self.string_idx', self.string_idx
        s = self.suffix_tree.string_list[self.string_idx]
        if self.last_char_idx <  self.first_char_idx:
            return s[self.first_char_idx:]
        return s[self.first_char_idx:self.last_char_idx +1]

    def __repr__(self):
        return 'Edge(%d,%d, %d:%s %s)' % (self.src_node_idx, self.dst_node_idx, self.first_char_idx,
            _infi(self.last_char_idx),  _q(self.get_string()))
            
def split_edge(edge, suffix, suffix_tree, string_idx):
    """Split edge to insert a new suffix. Old edge spans two edges after split."""
    #alloc new node
    new_node = Node()           #suffix.src_node_idx
    suffix_tree.nodes.append(new_node)
    new_node_idx = len(suffix_tree.nodes) - 1
    #alloc new edge

    new_edge = Edge(suffix_tree, new_node_idx, edge.dst_node_idx, string_idx, edge.first_char_idx + len(suffix), edge.last_char_idx)
    suffix_tree.insert_edge(new_edge, -1)
    #shorten existing edge
    edge.last_char_idx = edge.first_char_idx + len(suffix) - 1
    edge.dst_node_idx = new_node_idx
    return new_node_idx

class Suffix:
    """Suffix is represented by a node (as an index into a node list) and indices of first and last 
        character of suffix in containing string.
    """
    def __init__(self, src_node_idx, first_char_idx, last_char_idx):
        self.src_node_idx = src_node_idx
        self.first_char_idx = first_char_idx
        self.last_char_idx = last_char_idx
        #print '>', self, len(self)

    def is_explicit(self):
        return is_explicit_suffix(self)

    def is_implicit(self):
        return is_implicit_suffix(self)

    def canonize(self, suffix_tree, string_idx):
        canonize_suffix(self, suffix_tree, string_idx)

    def __repr__(self):
        return 'Suffix(%d,%d:%d"%s")' % (self.src_node_idx, self.first_char_idx, self.last_char_idx,
            self.suffix_tree.get_substring(self.first_char_idx, self.last_char_idx)) 

    def __len__(self):
        ln = self.last_char_idx - self.first_char_idx + 1
        return max(0, ln)

# There are 3 types of suffix,
#   Leaf        Ends in a terminator
#   Explicit    
#   Implicit        
def is_explicit_suffix(suffix):
    """Terminates on a node or end of a leaf"""
    return suffix.first_char_idx > suffix.last_char_idx

def is_implicit_suffix(suffix):
    return not is_explicit_suffix(suffix)

"""
    // A suffix in the tree is denoted by a Suffix structure
    // that denotes its last character.  The canonical
    // representation of a suffix for this algorithm requires
    // that the origin_node by the closest node to the end
    // of the tree.  To force this to be true, we have to
    // slide down every edge in our current path until we
    // reach the final node.

    void Suffix::Canonize()
    {
        if ( !Explicit() ) {
            Edge edge = Edge::Find( origin_node, T[ first_char_index ] );
            int edge_span = edge.last_char_index - edge.first_char_index;
            while ( edge_span <= ( last_char_index - first_char_index ) ) {
                first_char_index = first_char_index + edge_span + 1;
                origin_node = edge.end_node;
                if ( first_char_index <= last_char_index ) {
                   edge = Edge::Find( edge.end_node, T[ first_char_index ] );
                   edge_span = edge.last_char_index - edge.first_char_index;
                };
            }
        }
    }
"""
# Track this inner loops to see if it is O(N) as claimed
_ops_counts = {'num calls': 0, 'total loops': 0}
_ops_string_len = 1
_OPS_MULTIPLIER = 3
def _reset_ops_counts(string_len):
    global _ops_string_len
    global _ops_counts
    _ops_string_len = string_len
    _ops_counts = {'num calls': 0, 'total loops': 0}

def _count_op(key):
    global _ops_counts
    _ops_counts[key] += 1
    
def _ops_counts_strings():
    def get_one(key):
        return '%s=%7d (%3d%%)' % (key, _ops_counts[key], int(100.0 * _ops_counts[key]/_ops_string_len))
    return 'string len=%7d:{' % _ops_string_len + ','.join([get_one(key) for key in sorted(_ops_counts.keys())]) + '}'

def _check_ops_counts():
    if _ops_counts['total loops'] > _OPS_MULTIPLIER * _ops_string_len or \
        _ops_counts['num calls'] > _OPS_MULTIPLIER * _ops_string_len:
        print _ops_counts_strings()
    if _ops_counts['total loops'] > 1000:
        assert(_ops_counts['total loops'] <= _OPS_MULTIPLIER * _ops_string_len)
    if _ops_counts['num calls'] > 1000:    
        ssert(_ops_counts['num calls'] <= _OPS_MULTIPLIER * _ops_string_len)
  
def canonize_suffix(suffix, suffix_tree, string_idx):
    """ Ukkonen's algorithm requires that we work with these Suffix definitions in canonical form. 
        The Canonize() function is called to perform this transformation any time a Suffix object 
        is modified. The canonical representation of the suffix simply requires that the origin_node 
        in the Suffix object be the closest parent to the end point of the string.
    """
    string = suffix_tree.string_list[string_idx]
    _count_op('num calls')
    
    if False and _ops_counters['num calls'] % 100000 == 0:
        if suffix.first_char_idx < len(suffix_tree.string):
            first_char = suffix_tree.string[suffix.first_char_idx] 
        else:
            first_char = '***'
        print _ops_counts_strings(),[suffix.src_node_idx, first_char, suffix.first_char_idx]
    
    original_length = len(suffix)
    num_loops = 0
    edge_count = 0
    if not suffix.is_explicit():
        # Debug code !@#$
        if (suffix.src_node_idx, string[suffix.first_char_idx]) not in suffix_tree.edge_lookup.keys():
            print string_idx
            print len(suffix_tree.string_list)
        edge = suffix_tree.edge_lookup[suffix.src_node_idx, string[suffix.first_char_idx]]
        while len(edge) <= len(suffix):
            suffix.first_char_idx += len(edge)
            suffix.src_node_idx = edge.dst_node_idx
            if suffix.first_char_idx <= suffix.last_char_idx:
                edge = suffix_tree.edge_lookup[suffix.src_node_idx, string[suffix.first_char_idx]] 
                edge_count += 1        
            num_loops += 1
            _count_op('total loops')
            if False and num_loops % 10000 == 0:
                print 'num_loops = %7d,%7d,%8d' % (num_loops, edge_count, _ops_counters['total loops']), [suffix.src_node_idx, 
                    suffix_tree.string[suffix.first_char_idx]], original_length, len(suffix), \
                    [ord(c) for c in suffix_tree.string[suffix.first_char_idx:suffix.first_char_idx+20]], \
                    suffix.first_char_idx
                  
            assert(num_loops <= len(string))
    _check_ops_counts()

_TERMINATOR_BASE = 0x1000

class SuffixTree:
    def __init__(self):

        self.string_list = []
        self.terminator_list = []
        self.alphabet = set([])
        self.nodes = [Node()]
        self.edge_lookup = {} # by  source_node, first_char
        self.active_point = Suffix(0, 0, -1)

    def add_string(self, string, alphabet=None):
        _reset_ops_counts(len(string))
        
        terminator = unichr(_TERMINATOR_BASE + len(self.string_list))
        print 'terminator=%s' % [terminator]        

        string += terminator
        self.string_list.append(string)    
        
        if alphabet == None:
            alphabet = set(string)
        print 'alphabet=', len(alphabet), sorted(alphabet)
  
        self.alphabet |= alphabet    
 
        self.active_point = Suffix(0, 0, -1)
        for i in range(len(string)):
            print ' === add_prefix(%d) ===' % i
            add_prefix(i, self.active_point, self)
            show_all_suffixes(self)

        print '-' * 60
        print 'string added'
        print '-' * 60

    def insert_edge(self, edge, string_idx):
        string = self.string_list[string_idx]
        self.edge_lookup[edge.src_node_idx, string[edge.first_char_idx]] = edge

    def remove_edge(self, edge, string_idx):
        string = self.string_list[string_idx]
        del self.edge_lookup[edge.src_node_idx, string[edge.first_char_idx]]

    def get_substring(self, first_index, last_index):
        """Strings are typially first_index to last_index inclusive
            Otherwise they terminate at end of string"""
        if last_index >= first_index:
            return self.string[first_index : last_index+1]
        return self.string[first_index:]
 
    def find_substring(self, substring):
        """Returns the index of substring in string or -1 if it is not found."""
        if not substring:
            return -1
        print 'find_substring(%s)' % (substring)    
        #if self.case_insensitive:
        #    substring = substring.lower()
        curr_node = 0
        i = 0
        while i < len(substring):
            print '** i=%d, curr_node=%s, char=%s' % (i, curr_node, substring[i])
            edge = self.edge_lookup[curr_node, substring[i]]
            if not edge:
                print 'not an edge'
                return -1
            # ln is length of substring segment along current edge    
            ln = min(len(edge), len(substring) - i)
            print '  ', edge, len(edge), ln
            if substring[i:i + ln] != self.string_list[edge.string_idx][edge.first_char_idx:edge.first_char_idx + ln]:
                return -1
            i += len(edge)
            curr_node = edge.dst_node_idx
        print '   matching node=', curr_node 
        return edge.first_char_idx - len(substring) + ln

def add_prefix(last_char_idx, active_point, suffix_tree):
    """Add string prefix suffix_tree.string[:last_char_idx+1] to suffix tree
        active_point is a suffix """
    #print '***add_prefix(%d, %s)' % (last_char_idx, active_point)
    # Add prefix for the last string
    string_idx = len(suffix_tree.string_list) - 1
    string = suffix_tree.string_list[-1]
    last_parent_node_idx = -1
    
    while True:
        parent_node_idx = active_point.src_node_idx
        if active_point.is_explicit():
            if (active_point.src_node_idx, string[last_char_idx]) in suffix_tree.edge_lookup: 
                #already in tree
                break
        else:
            # edge of active_point
            edge = suffix_tree.edge_lookup[active_point.src_node_idx, string[active_point.first_char_idx]]
            if string[edge.first_char_idx + len(active_point)] == string[last_char_idx]: 
                #the given prefix is already in the tree, do nothing
                break
            else:
                parent_node_idx = edge.split(active_point, suffix_tree, string_idx)
        # Add new node and edge        
        suffix_tree.nodes.append(Node(-1))
        new_edge = Edge(suffix_tree, parent_node_idx, len(suffix_tree.nodes) - 1, string_idx, last_char_idx, _POSITIVE_INFINITY)##################
        suffix_tree.insert_edge(new_edge, -1)
        #add suffix link
        if last_parent_node_idx > 0:
            suffix_tree.nodes[last_parent_node_idx].suffix_link = parent_node_idx
        # update last_parent_node_idx and active_point for next loop    
        last_parent_node_idx = parent_node_idx
        if active_point.src_node_idx == 0:
            active_point.first_char_idx += 1
        else:
            active_point.src_node_idx = suffix_tree.nodes[active_point.src_node_idx].suffix_link
        active_point.canonize(suffix_tree, -1)
        
    if last_parent_node_idx > 0:
        suffix_tree.nodes[last_parent_node_idx].suffix_link = parent_node_idx
    #last_parent_node_idx = parent_node_idx
    active_point.last_char_idx += 1
    active_point.canonize(suffix_tree, -1)

#validation code
import collections
is_valid_suffix = collections.defaultdict(lambda: False)
branch_count = collections.defaultdict(lambda: 0)
def is_valid_suffix_tree(suffix_tree):
    walk_tree(suffix_tree, 0, {}, 0)
    for i in range(1, len(suffix_tree.string)):
        if not is_valid_suffix[i]:
            print 'not is_valid_suffix[%s]' % str(i)
            #return False
    leaf_sum = 0
    branch_sum = 0
    for i in range(len(suffix_tree.nodes)):
        if branch_count[i] == 0:
            print 'logic error'
            return False
        elif branch_count[i] == -1:
            leaf_sum += 1
        else:
            branch_sum += branch_count[i]
    if leaf_sum != len(suffix_tree.string):
        print 'leaf_sum != len(suffix_tree.string)'
        print 'leaf_sum:', leaf_sum
        return False
    if branch_sum != len(suffix_tree.nodes) - 1: #root dosn't have edge leading to it
        print 'branch_sum != len(suffix_tree.nodes) - 1'
        return False
    return True

def walk_tree(suffix_tree, current_node_idx, current_suffix, current_suffix_len):
    edges = 0
    for c in suffix_tree.alphabet:
        try:
            edge = suffix_tree.edge_lookup[current_node_idx, c]
            if current_node_idx != edge.src_node_idx:
                raise Exception('eeeeeeeeeeeeeeeeee!!!!!!!!!!!')
            print current_node_idx, edge.src_node_idx
            if branch_count[edge.src_node_idx] < 0:
                print 'error: node labeled as leaf has children!'
            branch_count[edge.src_node_idx] += 1
            edges += 1
            l = current_suffix_len
            for j in range(edge.first_char_idx, edge.last_char_idx + 1):
                current_suffix[l] = suffix_tree.string[j]
                l += 1
            if walk_tree(suffix_tree, edge.dst_node_idx, current_suffix, l):
                if branch_count[edge.dst_node_idx] > 0:
                    print 'error: leaf labeled as having children'
                branch_count[edge.dst_node_idx] -= 1 #leaves have '-1' children
        except KeyError:
            pass
    if edges == 0:
        #leaf. check suffix
        is_valid_suffix[current_suffix_len] = ''.join(current_suffix[i] for i in range(current_suffix_len)) == suffix_tree.string[-(current_suffix_len):]
        print ''.join(current_suffix[i] for i in range(current_suffix_len))
        if not is_valid_suffix[current_suffix_len]:
            print 'not is_valid_suffix[current_suffix_len]'
        ###########################################################
        return True
    else:
        return False

def show_edge(suffix_tree, src_node_idx, first_char):
    edge = suffix_tree.edge_lookup[src_node_idx, first_char]
    print edge
    print suffix_tree.string[edge.first_char_idx:edge.last_char_idx+1]

def show_node(node_idx):
    for c in ALPHABET:
        try:
            edge = suffix_tree.edge_lookup[node_idx, c]
            print edge
            print suffix_tree.string[edge.first_char_idx:edge.last_char_idx+1]
        except KeyError:
            pass
    print str(node_idx) + ' -> ' + str(suffix_tree.nodes[node_idx])

# !@#$ Peter code starts here    
def get_node_child_dict(suffix_tree):
    """Returns a dict of nodes and their children"""
    nodes = set([node_idx for node_idx,_ in suffix_tree.edge_lookup.keys()])
    node_dict = dict([(n,{}) for n in nodes])
    for key, edge in suffix_tree.edge_lookup.items():
        node_idx, first_char = key
        #print '>>', node_idx, first_char, edge, node_dict, node_dict.get(node_idx, [])
        node_dict[node_idx][first_char] = edge
       
    if False:
        for key in sorted(node_dict.keys()):
            print ' ', key 
            for first_char in sorted(node_dict[key]):
                print '  ', first_char, node_dict[key][first_char]
    return node_dict
    
def show_nodes_tree(suffix_tree):
    print 'show_nodes_tree' + '-' * 40
    node_dict = get_node_child_dict(suffix_tree)
    #print '+++++++++++++++' + '-' * 40
    for i, string in enumerate(suffix_tree.string_list):
        print ' string %3d: len=%4d %20s   <--- The string' % (i, len(string), _q(string))
    print '===============' + '-' * 40
    all_suffixes = [i+1 for i in range(len(string)) for string in suffix_tree.string_list]
    suffixes_found = [[]]
    
    for node_idx in sorted(node_dict.keys()):
        for first_char in sorted(node_dict[node_idx].keys()):
            print '> %4d %s : %s' % (node_idx, _q(first_char), node_dict[node_idx][first_char])
    print '***************' + '-' * 40
            
    def show_nodes(node_idx, indent, string):

        for first_char in sorted(node_dict[node_idx].keys()): 
            edge = node_dict[node_idx][first_char]
            new_string = string + edge.get_string()
            #print 'xxx', new_string
            #print 'yyy', first_char
            
            if edge.dst_node_idx in node_dict.keys():
                print ' %4s %20s %s%3d %s %-25s' % ('', '', '    ' * indent, node_idx, first_char, edge)
                show_nodes(edge.dst_node_idx, indent + 1, new_string)
            else:
                print ' %4d %20s %s%3d %s %-25s' % (len(new_string), _q(new_string), '    ' * indent, 
                    node_idx, _q(first_char), edge)  
                suffixes_found[0].append(len(new_string))

    show_nodes(0, 0, '')
    
    print '  all_suffixes:', len(all_suffixes), sorted(all_suffixes)    
    print 'suffixes_found:', len(suffixes_found[0]), sorted(suffixes_found[0])
    assert(sorted(suffixes_found[0]) == sorted(all_suffixes))
    print '+++++++++++++++' + '-' * 40

def show_all_suffixes(suffix_tree):
    print '-' * 20 + ' show_all_suffixes ' +  '-' * 20 
    print suffix_tree.string_list
    for key in sorted(suffix_tree.edge_lookup.keys()):
        print ' %s : %s' % (key, suffix_tree.edge_lookup[key])
    print '-' * 20 + ' ||||||||||||||||| ' + '-' * 20

def test(filename):
    def assert_test(string, substring):
        suffix_tree = SuffixTree(string)
        position = string.find(substring)
        idx = suffix_tree.find_substring(substring)
        if idx != position:
            print '   string = "%s"' % string
            print 'substring = "%s"' % substring
            print ' position = %d' % position
            print '      idx = %d' % idx 
        assert(idx == position)

    tests = [
        ('hello', 'lo'),
        ('a very long sentence', 'sentence')
    ]

    for string,substring in tests:
        assert_test(string, substring)

    lines_tested = 0
    lines = file(filename, 'rt').readlines()
    for ln in lines:
        string = ln.rstrip('\n')
        if string:
            substring = string[len(string)//3:2*len(string)//3]
            if substring:
                assert_test(string, substring)
                lines_tested += 1
                
    print '%d lines tested' % lines_tested


    
import random
import math    
def make_test_substrings(string, num, length):
    """Make a list of <num> substrings of length <lenght> of <string> as evenly divided as possible"""
    region_list = []
    n = len(string)
    for i in range(num):
        region_list.append({'start':int(i*n/num), 'end':int((i+1)*n/num), 'substrings':set([])})
        #print region_list[-1]
    #exit()    
    
    def add_substring(region):
        for i in range(1000//min(500,(len(region['substrings'])**2 + 1))):
            k = random.randint(region['start'], region['end'] - length)
            s = string[k: k + length]
            if string.find(s) > region['start'] and s not in region['substrings']:
                region['substrings'].add(s)
                break

    done = False            
    while not done:
        region_list.sort(key = lambda x: len(x['substrings']))
        for region in region_list:
            add_substring(region)
            if sum([len(x['substrings']) for x in region_list]) >= num:
                done = True
                break
        print num, sum([len(x['substrings']) for x in region_list]), [len(x['substrings']) for x in region_list[-10:]] 
                
    substrings = set(sum([list(x['substrings']) for x in region_list], []))
    #print len(substrings), substrings
    
    assert(len(substrings) == num)
    return sorted(substrings, key = lambda x: string.find(x))

def to_unicode(string):
    bytes = [ord(c) for c in string]
    unicd = u''.join([unichr(i) for i in bytes])
    return unicd
    
def test_generalized_suffix_tree(num_strings, length, num_repetitions):
    suffix_tree = SuffixTree()

    if False: # unit test for edge
        edge = Edge(suffix_tree, 0, 0, 0, 0, 0)
        print edge

    def make_string(length, n):
        return to_unicode(''.join([chr((ord('A')+i+n)%256) 
            for j in range(num_repetitions)
                for i in range(length) 
            ]))

    #string_list = [make_string(length, n*num_repetitions*length) for n in range(num_strings)]
    string_list = [make_string(length, 0) for n in range(num_strings)]

    for string in string_list:
        print 'string =', _q(string), '^' * 20
        suffix_tree.add_string(string)
        
    show_all_suffixes(suffix_tree)
    show_nodes_tree(suffix_tree)    

    for string in string_list:    
        substring = string[length//4:(3*length)//4]
        idx0 = string.find(substring)
        idx1 = suffix_tree.find_substring(substring)
        print 'idx0=%d,idx1=%d' % (idx0, idx1)
        print 'substring:  ', _q(substring)
        print ' original:', idx0, _q(string[idx0:idx0+len(substring)])
        print '   suffix:', idx1, _q(string[idx1:idx1+len(substring)])

def test_file_list(file_list):
    """Construct a generalized suffix tree whose strings are the contents of the files in 
        <file_list>
    """
    total_length = sum([os.path.getsize(filename) for filename in file_list])
    _reset_ops_counts(total_length)

    # Create tests
    if False:
        num_tests = 10
        length = 30
        substrings = []
        for filename in file_list:
            string = file(filename, 'rb').read()[:999999]
            string = to_unicode(string)
            substrings += make_test_substrings(string, num_tests, length)
        string = None
        #for i, ss in enumerate(substrings):
        #    print i, string.find(ss), ss
        #exit()
   
    # Create suffix tree
    suffix_tree = SuffixTree()
   
    for filename in file_list:
        string = file(filename, 'rb').read()[:999999]
        string = to_unicode(string)
        suffix_tree.add_string(string)
    string = None
    
    # Run tests
    for i in range(num_tests):
        substring = substrings[i]
        idx0 = string.find(substring)
        idx1 = suffix_tree.find_substring(substring)
        if idx0 == idx1:
            print 'idx0=%7d (%2d%%)'% (idx0, int(100.0 * idx0 / len(string))),
            if idx0 >= 0:
                print ', substring=%s' % _q(substring)
            else:
                print
        else:
            print 'idx0=%7d, idx1=%7d' % (idx0, idx1)
            if idx0 >= 0:
                print _q(substring)
                print _q(string[idx1:idx1+len(substring)])
        assert(substring == string[idx1:idx1+len(substring)])
        assert(idx0 == idx1)   

if __name__ == '__main__':
    test_generalized_suffix_tree(num_strings = 2, length = 3, num_repetitions=1)

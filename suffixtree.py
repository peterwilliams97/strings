"""
    I have lost the name of the website I copied the original version of this from. Will add it as
    soon as I find it.
    
    It will become a   
        Suffix Tree implementation based on http://marknelson.us/1996/08/01/suffix-trees/ 
"""
import string

POSITIVE_INFINITY = 1 << 30

class Node:
    def __init__(self, suffix_link=None):
        self.suffix_link = suffix_link

    def __repr__(self):
        return 'Node(%s)' % self.suffix_link

class Edge:
    """Links 2 nodes in a suffix tree"""
    def __init__(self, suffix_tree, src_node_idx, dst_node_idx, first_char_idx, last_char_idx):
        self.suffix_tree = suffix_tree          # For debugging !@#$
        self.src_node_idx = src_node_idx
        self.dst_node_idx = dst_node_idx
        self.first_char_idx = first_char_idx
        self.last_char_idx = last_char_idx
        #print self

    def split(self, suffix, suffix_tree):
        #print '  split(%s,%s)' % (self, suffix)
        return split_edge(self, suffix, suffix_tree)

    def __len__(self):
        return self.last_char_idx - self.first_char_idx + 1
        
    def get_string(self):
        return self.suffix_tree.string[self.first_char_idx:self.last_char_idx +1]

    def __repr__(self):
        return 'Edge(%d,%d, %d:%d "%s")' % (self.src_node_idx, self.dst_node_idx, self.first_char_idx,
            self.last_char_idx, self.get_string())

def split_edge(edge, suffix, suffix_tree):
    """Split edge to insert a new suffix. Old edge spans two edges after split."""
    #alloc new node
    new_node = Node()           #suffix.src_node_idx
    suffix_tree.nodes.append(new_node)
    new_node_idx = len(suffix_tree.nodes) - 1
    #alloc new edge
    new_edge = Edge(suffix_tree, new_node_idx, edge.dst_node_idx, edge.first_char_idx + len(suffix), edge.last_char_idx)
    suffix_tree.insert_edge(new_edge)
    #shorten existing edge
    edge.last_char_idx = edge.first_char_idx + len(suffix) - 1
    edge.dst_node_idx = new_node_idx
    return new_node_idx

class Suffix:
    """Suffix is represented by a node (as an index into a node list) and indices of first and last 
        character of suffix in containing string.
    """
    def __init__(self, suffix_tree, src_node_idx, first_char_idx, last_char_idx):
        self.suffix_tree = suffix_tree      # For debugging
        self.src_node_idx = src_node_idx
        self.first_char_idx = first_char_idx
        self.last_char_idx = last_char_idx
        #print '>', self, len(self)

    def is_explicit(self):
        return is_explicit_suffix(self)

    def is_implicit(self):
        return is_implicit_suffix(self)

    def canonize(self, suffix_tree):
        canonize_suffix(self, suffix_tree)

    def __repr__(self):
        return 'Suffix(%d,%d:%d"%s")' % (self.src_node_idx, self.first_char_idx, self.last_char_idx,
            self.suffix_tree.get_substring(self.first_char_idx, self.last_char_idx)) 

    def __len__(self):
        ln = self.last_char_idx - self.first_char_idx + 1
        return max(0, ln)

# There are 3 types of suffix,
#   Leaf
#   Explicit
#   Implicit        
def is_explicit_suffix(suffix):
    """Terminates on a node or end of a leaf"""
    return suffix.first_char_idx > suffix.last_char_idx

def is_implicit_suffix(suffix):
    return not is_explicit_suffix(suffix)

if False:    
    def _canonize_suffix(suffix, suffix_tree, depth =0):
        if not suffix.is_explicit():
            edge = suffix_tree.edge_lookup[suffix.src_node_idx, suffix_tree.string[suffix.first_char_idx]]
            if len(edge) <= len(suffix):
                suffix.first_char_idx += len(edge)
                suffix.src_node_idx = edge.dst_node_idx
                canonize_suffix(suffix, suffix_tree, depth + 1)
else:
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
    num_calls = 0
    def canonize_suffix(suffix, suffix_tree):
        """ Ukkonen's algorithm requires that we work with these Suffix definitions in canonical form. 
            The Canonize() function is called to perform this transformation any time a Suffix object 
            is modified. The canonical representation of the suffix simply requires that the origin_node 
            in the Suffix object be the closest parent to the end point of the string.
        """
        global num_calls
        num_calls += 1
        if num_calls % 10000 == 0:
            if suffix.first_char_idx < len(suffix_tree.string):
                first_char = suffix_tree.string[suffix.first_char_idx] 
            else:
                first_char = '***'
            print 'num_calls = %7d len = %7d (%3d%%)' % (num_calls, len(suffix_tree.string), 
                int(100.0 * num_calls/len(suffix_tree.string))), \
                [suffix.src_node_idx, first_char, suffix.first_char_idx]
        original_length = len(suffix)        
        num_loops = 0
        edge_count = 0
        if not suffix.is_explicit():
            edge = suffix_tree.edge_lookup[suffix.src_node_idx, suffix_tree.string[suffix.first_char_idx]]
            while len(edge) <= len(suffix):
                suffix.first_char_idx += len(edge)
                suffix.src_node_idx = edge.dst_node_idx
                if suffix.first_char_idx <= suffix.last_char_idx:
                    edge = suffix_tree.edge_lookup[suffix.src_node_idx, suffix_tree.string[suffix.first_char_idx]] 
                    edge_count += 1        
                num_loops += 1
                if num_loops % 10000 == 0:
                    print 'num_loops = %7d' % num_loops, edge_count, [suffix.src_node_idx, 
                        suffix_tree.string[suffix.first_char_idx]], original_length, len(suffix), \
                        [ord(c) for c in suffix_tree.string[suffix.first_char_idx:suffix.first_char_idx+20]], \
                        suffix.first_char_idx
                      
                assert(num_loops <= len(suffix_tree.string))

class SuffixTree:
    def __init__(self, string, alphabet=None):
       
        if alphabet == None:
            alphabet = set(string)
        print 'alphabet=', len(alphabet), sorted(alphabet)
        for i in range(256):
            if chr(i) not in sorted(alphabet):
                terminator = chr(i)
                break
        print 'terminator=', terminator        
        string += terminator        
        alphabet.add(terminator)
        if len(alphabet) > 200:
            exit()
        self.string = string    
        self.alphabet = alphabet    
        self.nodes = [Node()]
        self.edge_lookup = {} # by  source_node, first_char

        self.active_point = Suffix(self, 0, 0, -1)
        for i in range(len(string)):
            add_prefix(i, self.active_point, self)

    def insert_edge(self, edge):
        self.edge_lookup[edge.src_node_idx, self.string[edge.first_char_idx]] = edge

    def remove_edge(self, edge):
        del self.edge_lookup[edge.src_node_idx, self.string[edge.first_char_idx]]

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
        #print 'find_substring(%s, %s)' % (self.string, substring)    
        #if self.case_insensitive:
        #    substring = substring.lower()
        curr_node = 0
        i = 0
        while i < len(substring):
            #print '** i=%d, curr_node=%s, char=%s' % (i, curr_node, substring[i])
            edge = self.edge_lookup[curr_node, substring[i]]
            if not edge:
                print 'not an edge'
                return -1
            # ln is length of substring segment along current edge    
            ln = min(len(edge), len(substring) - i)
            #print '  ', edge, len(edge), ln
            if substring[i:i + ln] != self.string[edge.first_char_idx:edge.first_char_idx + ln]:
                return -1
            i += len(edge)
            curr_node = edge.dst_node_idx
        #print '   matching node=', curr_node 
        return edge.first_char_idx - len(substring) + ln

def add_prefix(last_char_idx, active_point, suffix_tree):
    """Add string prefix suffix_tree.string[:last_char_idx+1] to suffix tree
        active_point is a suffix """
    #print '***add_prefix(%d, %s)' % (last_char_idx, active_point)
    last_parent_node_idx = -1
    while True:
        parent_node_idx = active_point.src_node_idx
        if active_point.is_explicit():
            if (active_point.src_node_idx, suffix_tree.string[last_char_idx]) in suffix_tree.edge_lookup: 
                #already in tree
                break
        else:
            # edge of active_point
            edge = suffix_tree.edge_lookup[active_point.src_node_idx, suffix_tree.string[active_point.first_char_idx]]
            if suffix_tree.string[edge.first_char_idx + len(active_point)] == suffix_tree.string[last_char_idx]: 
                #the given prefix is already in the tree, do nothing
                break
            else:
                parent_node_idx = edge.split(active_point, suffix_tree)
        # Add new node and edge        
        suffix_tree.nodes.append(Node(-1))
        new_edge = Edge(suffix_tree, parent_node_idx, len(suffix_tree.nodes) - 1, last_char_idx, POSITIVE_INFINITY)##################
        suffix_tree.insert_edge(new_edge)
        #add suffix link
        if last_parent_node_idx > 0:
            suffix_tree.nodes[last_parent_node_idx].suffix_link = parent_node_idx
        # update last_parent_node_idx and active_point for next loop    
        last_parent_node_idx = parent_node_idx
        if active_point.src_node_idx == 0:
            active_point.first_char_idx += 1
        else:
            active_point.src_node_idx = suffix_tree.nodes[active_point.src_node_idx].suffix_link
        active_point.canonize(suffix_tree)
        
    if last_parent_node_idx > 0:
        suffix_tree.nodes[last_parent_node_idx].suffix_link = parent_node_idx
    #last_parent_node_idx = parent_node_idx
    active_point.last_char_idx += 1
    active_point.canonize(suffix_tree)

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
    print '+++++++++++++++' + '-' * 40
    print ' %4d %20s' % (len(suffix_tree.string), suffix_tree.string)
    all_suffixes = set([i+1 for i in range(len(suffix_tree.string))])
    suffixes_found = [set([])]

    def show_nodes(node_idx, indent, string):
    
        for first_char in sorted(node_dict[node_idx].keys()): 
            edge = node_dict[node_idx][first_char]
            new_string = string + edge.get_string()
            
            if edge.dst_node_idx in node_dict.keys():
                print ' %4s %20s %s%3d %s %-25s' % ('', '', '    ' * indent, node_idx, first_char, edge)
                show_nodes(edge.dst_node_idx, indent + 1, new_string)
            else:
                print ' %4d %20s %s%3d %s %-25s' % (len(new_string), new_string, '    ' * indent, node_idx, first_char, edge)
                suffixes_found[0].add(len(new_string))

    show_nodes(0, 0, '')
    
    assert(suffixes_found[0] == all_suffixes)
    print '+++++++++++++++' + '-' * 40

def show_all_suffixes(suffix_tree):
    print 'suffix_tree.edge_lookup' + '-' * 60
    print suffix_tree.string
    for key in sorted(suffix_tree.edge_lookup.keys()):
        print ' %s : %s' % (key, suffix_tree.edge_lookup[key]) 
    
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
    
if __name__ == '__main__':
    import sys
    if False:
        test(sys.argv[0])
        test_str = 'abaababaabaab$'#'mississippi$'
        test_str = 'abcab$'
        if len(sys.argv) < 2:
            print 'usage: python %s <string> <substring>' % sys.argv[0]
            exit()
        
    if False:    
        test_str = sys.argv[1]
        substring = sys.argv[2]
    else:
        test_str = file(sys.argv[1], 'rb').read()[:999999]
        substring = test_str[len(test_str)//3:2*len(test_str)//3]
    
    POSITIVE_INFINITY = len(test_str) - 1
    suffix_tree = SuffixTree(test_str)
    is_valid = is_valid_suffix_tree(suffix_tree)
    print 'is_valid_suffix_tree:', is_valid

    show_all_suffixes(suffix_tree)
    show_nodes_tree(suffix_tree)
    
    idx = suffix_tree.find_substring(substring)
    print 'idx=%d' % idx
    if idx >= 0:
        print substring
        print test_str[idx:idx+len(substring)]
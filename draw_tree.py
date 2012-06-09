from __future__ import division
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.transforms as mtransforms
from numpy.random import random_integers as rnd
import csv
import math
import sys

def read_tree_data(tree_path):
    """Read a tab delimited file with columns 
        Start 	End 	Suf 	First 	Last 	String"""

    reader = csv.reader(open(tree_path, 'rb'), delimiter=',', quotechar='"')
    header = [s.strip() for s in reader.next()]
    print header, len(header)
    data = []
    for i,row in enumerate(reader):
        if not len(row):
            break
        data.append([row[0]] + [int(x) for x in row[1:-1]] + [row[-1]])
    columns = zip(*data)    
    return dict(zip(header[1:],columns[1:]))

def get_order(a_list):
    pairs = [(i,x) for (i,x) in enumerate(a_list)]
    pairs.sort(key = lambda x: x[1])
    order = [x[0] for x in pairs]
    return order

DISPLAY_KEYS = ['Start', 'End', 'Suf', 'First', 'Last', 'String']
def get_row(tree_dict, i):
    return [tree_dict[key][i] for key in DISPLAY_KEYS]

class Edge:
    def __init__(self, start_node, end_node, suffix, first, last, string):
        self.start_node = start_node
        self.end_node = end_node
        self.suffix = suffix
        self.first = first
        self.last = last
        self.string = string
        
    def __repr__(self):
        return 'start=%d,end=%d,suffix=%d,first=%d,last=%d,string=%s' % (self.start_node,
            self.end_node, self.suffix, self.first, self.last, self.string)

class Node:
    def __init__(self, parent):
        self.parent = parent
        self.children = []
        
    def add_child(self, edge_idx):
        self.children.append(edge_idx) 

    def __repr__(self):
        return 'parent=%s,children=%s' % (self.parent, self.children)

def parse_tree_data(tree_dict):
    
    n = len(tree_dict.values()[0])
    sort_dict = [(tree_dict['Start'][i], tree_dict['End'][i], tree_dict['String'][i][0]) for i in range(n)]
    order = get_order(sort_dict)

    # Some debug info
    print 'keys:', tree_dict.keys()
    print 'tree_dict stats: % rows' % n

    for key in DISPLAY_KEYS[:-1]:
        assert(key in tree_dict.keys())
        print '%10s : min = %3d, max = %3d' % (key, min(tree_dict[key]), max(tree_dict[key]))

    #print '\t'.join(DISPLAY_KEYS)
    #for i in order:
    #    print '\t'.join([str(tree_dict[key][i]) for key in DISPLAY_KEYS])

    # Run through edges
   
    edge_list = []
    node_dict = {}
    start_node_idx = tree_dict['Start'][0]
    node = Node(start_node_idx)
    node_dict[start_node_idx] = node
    for i in order:
        edge = Edge(*get_row(tree_dict, i))
        edge_list.append(edge)
        if tree_dict['Start'][i] != start_node_idx:
            start_node_idx = tree_dict['Start'][i]
            node = Node(start_node_idx)
            node_dict[start_node_idx] = node
        # Add outgoing edges    
        node.add_child(i)
        
    return edge_list, node_dict    

class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        
    def __repr__(self):
        return '(%.1f,%.1f)' % (self.x, self.y)
        
    def __add__(val1, val2):
        return Point(val1.x + val2.x, val1.y + val2.y)
        
    def __sub__(val1, val2):
        return Point(val1.x - val2.x, val1.y - val2.y)
    
    def __mul__(val, fac):
        return Point(val.x*fac, val.y*fac)
        
    def get_coords(self):
        return [self.x, self.y]
 
if False:        
    p1 = Point(1,1)
    p2 = Point(2,2)
    p3 = p1 + p2
    p4 = p3 * 2
    print p1
    print p2
    print p3
    print p4
    exit()  
        
class Line():
    def __init__(self, pt1, pt2, string = None):
        self.pt1 = pt1
        self.pt2 = pt2
        self.string = string
    
    def get_angle(self):
        angle = math.atan2(*(self.pt2 - self.pt1).get_coords())
        print 'get_angle: pt2=%s, pt1=%s, diff=%s, angle=%.1f=%d' % (self.pt2, self.pt1, 
            self.pt2 - self.pt1, angle, int(angle/math.pi*180.0))
        return angle
     
    def get_x(self):
        return [self.pt1.x, self.pt2.x]
    
    def get_y(self):
        return [self.pt1.y, self.pt2.y]
        
    def get_points(self):
        return [self.pt1, self.pt2]

def make_points_random(number, width, height):
    points = []
    for i in range(number):
        x = rnd(1,width-1)/width
        y = rnd(1,height-1)/height
        points.append(Point(x,y))
    return points

EDGE_LEN = 1.0
    
def make_points(edge_list, node_dict):
    root_x = 0.0
    root_y = 0.0
    points = []

    def make_node_points(node, x, y, depth):
        gap = ' ' * depth
        print gap + 'node=%s' % (node)
        points.append(Point(x,y))
        dtheta = math.pi / (len(node.children) + 2)
        for i, child_idx in enumerate(node.children):
            angle = (i+1)*dtheta
            dx = EDGE_LEN * math.cos(angle)
            dy = EDGE_LEN * math.sin(angle)
            edge = edge_list[child_idx]
            print gap + ' edge %2d=%s' % (i, edge)
            if edge.end_node in node_dict.keys():
                child_node = node_dict[edge.end_node]
                make_node_points(child_node, x+dx, y+dy, depth + 1)
                # retrace
                points.append(Point(x,y))

    make_node_points(node_dict[0], root_x, root_y, 0)
    return points     

def make_lines(edge_list, node_dict):
    start_pt = Point(0.0, 0.0)
    lines = []

    def make_node_lines(node, string, pt0, pt, depth):
        gap = ' ' * depth
        #print gap + 'node=%s' % (node)
        if pt0:
            lines.append(Line(pt0, pt, string))
        dtheta = math.pi / (len(node.children) + 2)
        for i, child_idx in enumerate(node.children):
            angle = (i+1)*dtheta
            #dx = EDGE_LEN * math.cos(angle)
            #dy = math.sin(angle)
            dxy = Point(math.cos(angle), math.sin(angle)) * EDGE_LEN  
            pt1 = pt + dxy
            edge = edge_list[child_idx]
            #print gap + ' edge %2d=%s' % (i, edge)
            if edge.end_node in node_dict.keys():
                child_node = node_dict[edge.end_node]
                make_node_lines(child_node, edge.string, pt, pt1, depth + 1)

    make_node_lines(node_dict[0], None, None, start_pt, 0)
    for ln in lines:
        ln.pt1.y = -ln.pt1.y
        ln.pt2.y = -ln.pt2.y
    return lines    

def get_min_max(a_list):
    F = 0.1
    mi = min(a_list)
    ma = max(a_list)
    d = ma - mi
    return mi - d * F, ma + d * F
    
def scale_min_max(lines, min_x, max_x, min_y, max_y):
    dx = 1.0/(max_x - min_x)
    dy = 1.0/(max_y - min_y)
    def scale(pt):
        return Point((pt.x - min_x) * dx, (pt.y - min_y) * dy)
    return [Line(scale(ln.pt1), scale(ln.pt2), ln.string) for ln in lines]  

def draw_tree(points): 
    fig1 = plt.figure()
    ax = fig1.add_axes([0.1, 0.1, 0.8, 0.8])

    points_x = [pt.x for pt in points]
    points_y = [-pt.y for pt in points]
    
    # draw lines
    l, = ax.plot(points_x, points_y, "bo-", mec="b", lw=2, ms=10, label="Line 1")
    ax.text(1, 1.6, tex, fontsize=20, va='bottom')
   
    # draw shadows with same lines with slight offset and gray colors.
    if False:
        xx = l.get_xdata()
        yy = l.get_ydata()
        shadow, = ax.plot(xx, yy)
        shadow.update_from(l)

        # adjust color
        shadow.set_color("0.2")
        # adjust zorder of the shadow lines so that it is drawn below the
        # original lines
        shadow.set_zorder(l.get_zorder()-0.5)

        # offset transform
        ot = mtransforms.offset_copy(l.get_transform(), fig1,   x=4.0, y=-6.0, units='points')

        shadow.set_transform(ot)

        # set the id for a later use
        shadow.set_gid(l.get_label()+"_shadow")
        

    ax.set_xlim(*get_min_max(points_x))
    ax.set_ylim(*get_min_max(points_y))

    if False:    
        
        # save the figure as a string in the svg format.
        from StringIO import StringIO
        f = StringIO()
        plt.savefig(f, format="svg")

    plt.show()
    
def draw_tree_lines(lines): 
    fig1 = plt.figure()
    ax = fig1.add_axes([0.1, 0.1, 0.8, 0.8])
 
    points_x = sum([ln.get_x() for ln in lines],[])
    points_y = sum([ln.get_y() for ln in lines],[])
    min_x, max_x = get_min_max(points_x)
    min_y, max_y =get_min_max(points_y)
    lines = scale_min_max(lines, min_x, max_x, min_y, max_y)
    
    # draw lines
    for i, ln in enumerate(lines):
        plt.plot(ln.get_x(), ln.get_y(), "bo-", mec="b", lw=2, ms=10, label=ln.string)
        rotation = (90.0 - 180.0 * ln.get_angle() / math.pi)
        bbox = dict(boxstyle='square', fc=(1.0, 1.0, 1.0),
            ec=(0.1, 0.1, 0.1), )
        mid = (ln.pt1 + ln.pt2) * 0.5
        plt.text(mid.x, mid.y, '-- ' + ln.string + ' --', size=8, rotation=rotation,
                ha='center', va='center',
                bbox = bbox
            ) 
    
    ax.set_xlim(0.0, 1.0)
    ax.set_ylim(0.0, 1.0)
    #ax.set_xlim(*get_min_max(points_x))
    #ax.set_ylim(*get_min_max(points_y))
    plt.show()    
    
if __name__ == '__main__':

    if False:
        width = 10
        height = 10
        number = 10
        points = make_points(number, width, height)
        draw_tree(points, width, height)

    if len(sys.argv) <= 1:
        print 'Usage: python draw_tree <tree file>'
        exit()

    tree_dict = read_tree_data(sys.argv[1])    
    edge_list, node_dict = parse_tree_data(tree_dict)

    if False:
        points = make_points(edge_list, node_dict)
        print 'number of points', len(points)
        min_x = min([pt.x for pt in points])
        min_y = min([pt.y for pt in points])
        max_x = max([pt.x for pt in points])
        max_y = max([pt.y for pt in points])
        print 'min_x, min_y', (min_x, min_y)
        print 'max_x, max_y', (max_x, max_y) 
        draw_tree(points)

    lines = make_lines(edge_list, node_dict)
    print 'number of lines', len(lines)

    draw_tree_lines(lines)
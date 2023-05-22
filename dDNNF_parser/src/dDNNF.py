class Edge:
    def __init__(self, target, consts, free):
        self.target = target
        self.consts = consts
        self.free = free

class Node:
    def __init__(self, num):
        self.num = num
        self.mc = -1

    def add_child(self, target, consts, free):
        pass

    def annotate_mc(self):
        pass

class AndNode(Node):
    def __init__(self, num):
        Node.__init__(self, num)
        self.children = {}

    def add_child(self, target, consts, free):
        if len(consts) != 0 or len(free) != 0:
            raise ValueError('AndNode children do not have consts or free variables')

        self.children[target.num] = Edge(target, consts, free)

    def annotate_mc(self):
        self.mc = 1

        if len(self.children) == 0:
            self.mc = 0

        for i in self.children:
            self.mc *= self.children[i].target.mc


class OrNode(Node):
    def __init__(self, num):
        Node.__init__(self, num)
        self.children = {}

    def add_child(self, target, consts, free):
        self.children[target.num] = Edge(target, consts, free)

    def annotate_mc(self):
        self.mc = 0

        for i in self.children:
            self.mc += self.children[i].target.mc * (2**len(self.children[i].free))

class UnaryNode(Node):
    def __init__(self, num):
        Node.__init__(self, num)
        self.child = None

    def add_child(self, target, consts, free):
        self.child = Edge(target, consts, free)

    def annotate_mc(self):
        self.mc = self.child.target.mc * (2**len(self.child.free))

class TrueNode(Node):
    def __init__(self, num):
        Node.__init__(self, num)
        self.mc = 1

    def add_child(self, target, consts, free):
        raise ValueError('Cannot add child to TrueNode')

    def annotate_mc(self):
        self.mc = 1

class FalseNode(Node):
    def __init__(self, num):
        Node.__init__(self, num)
        self.mc = 0

    def add_child(self, target, consts, free):
        raise ValueError('Cannot add child to FalseNode')

    def annotate_mc(self):
        self.mc = 0

class dDNNF:
    def __init__(self):
        self.nodes = []

    def add_node(self, num, c):
        numt = len(self.nodes) + 1
        if num != numt:
            raise ValueError(f'node id {num} and calculated id {numt} do not match')

        self.nodes.append(c(num))
        return num

    def get_node(self, num):
        return self.nodes[num - 1]

def from_file(path):
    res = dDNNF()

    with open(path, 'r') as file:
        for line in file:
            line = line.strip()

            if line.startswith('o '):
                num = int(line.split(' ')[1])
                res.add_node(num, OrNode)

            elif line.startswith('a '):
                num = int(line.split(' ')[1])
                res.add_node(num, AndNode)

            elif line.startswith('u '):
                num = int(line.split(' ')[1])
                res.add_node(num, UnaryNode)

            elif line.startswith('t '):
                num = int(line.split(' ')[1])
                res.add_node(num, TrueNode)

            elif line.startswith('f '):
                num = int(line.split(' ')[1])
                res.add_node(num, FalseNode)

            elif not line.startswith('c '):
                tmp = line.split(';')
                p1 = tmp[0].split(' ')

                src = int(p1[0])
                dst = int(p1[1])

                cnst = [int(x) for x in p1[2:] if x != '' and x != '0']
                free = []

                if len(tmp) == 2:
                    p2 = tmp[1].split(' ')
                    free = [int(x) for x in p2 if x != '' and x != '0']

                # print(f"    {src} -> {dst}: {cnst} ; {free}")
                src = res.get_node(src)
                dst = res.get_node(dst)

                src.add_child(dst, cnst, free)




    return res


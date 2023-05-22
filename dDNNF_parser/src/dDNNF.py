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

        self.children[target] = Edge(target, consts, free)

    def annotate_mc(self):
        self.mc = 1

        if len(self.children) == 0:
            self.mc = 0

        for i in self.children:
            self.mc *= self.children[i].target.mc


class OrNode(Node):
    def __init__(self, num):
        Node.__init__(self, num)
        self.children = []

    def add_child(self, target, consts, free):
        self.children[target] = Edge(target, consts, free)

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
    pass

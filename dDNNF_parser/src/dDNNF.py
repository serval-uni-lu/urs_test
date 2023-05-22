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

    def get_childrend_ids(self):
        pass

    def get_vars(self):
        pass

    def print_dot(self):
        pass

class AndNode(Node):
    def __init__(self, num):
        Node.__init__(self, num)
        self.children = []

    def add_child(self, target, consts, free):
        if len(consts) != 0 or len(free) != 0:
            raise ValueError('AndNode children do not have consts or free variables')

        self.children.append(Edge(target, consts, free))

    def annotate_mc(self):
        self.mc = 1

        if len(self.children) == 0:
            self.mc = 0

        for i in self.children:
            self.mc *= i.target.mc

        if len(self.children) == 0:
            self.mc = 0

    def get_childrend_ids(self):
        return [c.target.num for c in self.children]

    def get_vars(self):
        res = set()

        for c in self.children:
            res.update(c.free)
            res.update([abs(x) for x in c.consts])
        return res

    def print_dot(self):
        print(f"\"{self.num}\" [label=\"A {self.num}: {self.mc}\"];")


class OrNode(Node):
    def __init__(self, num):
        Node.__init__(self, num)
        self.children = []

    def add_child(self, target, consts, free):
        self.children.append(Edge(target, consts, free))

    def annotate_mc(self):
        self.mc = 0

        for i in self.children:
            self.mc += i.target.mc * (2**len(i.free))

        if len(self.children) == 0:
            self.mc = 1

    def get_childrend_ids(self):
        return [c.target.num for c in self.children]

    def get_vars(self):
        res = set()

        for c in self.children:
            res.update(c.free)
            res.update([abs(x) for x in c.consts])
        return res

    def print_dot(self):
        print(f"\"{self.num}\" [label=\"O {self.num}: {self.mc}\"];")

class UnaryNode(Node):
    def __init__(self, num):
        Node.__init__(self, num)
        self.child = None

    def add_child(self, target, consts, free):
        self.child = Edge(target, consts, free)

    def annotate_mc(self):
        self.mc = self.child.target.mc * (2**len(self.child.free))

    def get_childrend_ids(self):
        return [self.child.target.num]

    def get_vars(self):
        tmp = set([abs(x) for x in self.child.consts])
        tmp.update(self.child.free)
        return tmp

    def print_dot(self):
        print(f"\"{self.num}\" [label=\"U {self.num}: {self.mc}\"];")

class TrueNode(Node):
    def __init__(self, num):
        Node.__init__(self, num)
        self.mc = 1

    def add_child(self, target, consts, free):
        raise ValueError('Cannot add child to TrueNode')

    def annotate_mc(self):
        self.mc = 1

    def get_childrend_ids(self):
        return []

    def get_vars(self):
        return set()

    def print_dot(self):
        print(f"\"{self.num}\" [label=\"T {self.num}: {self.mc}\"];")


class FalseNode(Node):
    def __init__(self, num):
        Node.__init__(self, num)
        self.mc = 0

    def add_child(self, target, consts, free):
        raise ValueError('Cannot add child to FalseNode')

    def annotate_mc(self):
        self.mc = 0

    def get_childrend_ids(self):
        return []

    def get_vars(self):
        return set()

    def print_dot(self):
        print(f"\"{self.num}\" [label=\"F {self.num}: {self.mc}\"];")

class dDNNF:
    def __init__(self):
        self.nodes = []
        self.ordering = []

    def add_node(self, num, c):
        numt = len(self.nodes) + 1
        if num != numt:
            raise ValueError(f'node id {num} and calculated id {numt} do not match')

        self.nodes.append(c(num))
        return num

    def get_node(self, num):
        return self.nodes[num - 1]

    def compute_ordering(self):
        self.ordering = []
        stack = [1]
        visited = set()

        while len(stack) != 0:
            # num = stack.pop()
            num = stack[len(stack) - 1]
            if num not in visited:
                n = self.get_node(num)

                all_done = True
                for c in n.get_childrend_ids():
                    if c not in visited:
                        all_done = False
                        stack.append(c)

                if all_done:
                    stack.pop()
                    visited.add(num)
                    self.ordering.append(num)
            else:
                stack.pop()
                pass

    def annotate_mc(self):
        for i in self.ordering:
            self.get_node(i).annotate_mc()

    def get_vars(self):
        res = set()

        for i in self.ordering:
            res.update(self.get_node(i).get_vars())

        return res

    def print_dot(self):
        print("digraph g {")
        for i in self.ordering:
            n = self.get_node(i)
            n.print_dot()
            # print(f"\"{i}\" [label=\"{i}: {n.mc}\"];")

            for c in n.get_childrend_ids():
                print(f"\"{i}\" -> \"{c}\";")

        print("}")

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

                src = res.get_node(src)
                dst = res.get_node(dst)

                src.add_child(dst, cnst, free)

    res.compute_ordering()

    return res


from subprocess import getoutput
import random
import argparse
import os
import sys

import dDNNF

# def make_temp_name(dir = tempfile.gettempdir()):
#     return os.path.join(dir, str(uuid.uuid1()))

def compute_dDNNF(cnf):
    # tmp = make_temp_name()
    tmp = cnf + ".nnf"
    if not os.path.exists(tmp):
        D4_cmd = '/d4 \"{}\" -dDNNF -out=\"{}\" 2>&1'
        r = getoutput(D4_cmd.format(cnf, tmp))
    return tmp

def sample(nnf):
    stack = [nnf.get_node(1)]
    sample = []

    while len(stack) > 0:
        # cid = stack.pop()
        node = stack.pop()

        if type(node) is dDNNF.OrNode:
            key = random.randint(1, node.mc)

            for c in node.children:
                tmc = c.target.mc * (2**len(c.free))

                if key <= tmc:
                    for i in c.free:
                        sample.append(random.choice([i, -i]))
                    for i in c.consts:
                        sample.append(i)

                    stack.append(c.target)
                    break
                else:
                    key -= tmc

        elif type(node) is dDNNF.AndNode:
            for c in node.children:
                if c.target.mc > 0:
                    stack.append(c.target)

        elif type(node) is dDNNF.UnaryNode:
            stack.append(node.child.target)

            for i in node.child.free:
                sample.append(random.choice([i, -i]))

            for i in node.child.consts:
                sample.append(i)

        elif type(node) is dDNNF.TrueNode:
            pass
        elif type(node) is dDNNF.FalseNode:
            raise ValueError('encountered FalseNode while sampling')

    return sample

parser = argparse.ArgumentParser()
parser.add_argument("-c", "--cnf", type=str, help="path to the cnf formula")
# parser.add_argument("-k", type=int, default=50)
parser.add_argument("-n", type=int, default=100, help="number of samples")
parser.add_argument("-s", type=int, default=0, help="rng seed")

args = parser.parse_args()
cnf_file = args.cnf

if args.s != 0:
    random.seed(args.s)

dDNNF_path = compute_dDNNF(cnf_file)
nnf = dDNNF.from_file(dDNNF_path)
nnf.annotate_mc()

print(f"c {dDNNF_path}")
print(f"c mc {nnf.get_node(1).mc}", file=sys.stderr)

for _ in range(0, args.n):
    s = sample(nnf)
    s.sort(key=abs)
    print(" ".join(map(str, s)))
    # print(" ".join(map(str, s)), file=sys.stderr)

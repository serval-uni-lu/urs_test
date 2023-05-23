from subprocess import getoutput
import os
import argparse
import dDNNF

parser = argparse.ArgumentParser()
parser.add_argument("-f", "--file", type=str)

args = parser.parse_args()

def get_mc(cnf):
    D4_cmd = 'd4_mod \"{}\" -dDNNF -out=\"{}.nnf\" 2>&1 | grep -E \'^s [0-9]+$\' | sed \'s/^s //g\''
    r = getoutput(D4_cmd.format(cnf, cnf))
    return int(r)

orig_mc = get_mc(args.file)
dDNNF_file = args.file + ".nnf"

res = dDNNF.from_file(dDNNF_file)
res.annotate_mc()

print(f"{args.file}, {res.get_node(1).mc}, {orig_mc}")
os.unlink(dDNNF_file)
# print(res.get_node(1).mc)
# print("")
# print(res.get_node(1).mc_by_var)
# res.print_dot()

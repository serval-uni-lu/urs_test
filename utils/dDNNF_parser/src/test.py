from subprocess import getoutput
import os
import argparse

import dDNNF
import DIMACS

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

if orig_mc != res.get_node(1).mc:
    print(f"error in {args.file} , true mc: {orig_mc}, got {res.get_node(1).mc}")

if orig_mc != res.get_node(1).mc_by_var[0]:
    print(f"error in {args.file} , true mc: {orig_mc}, got (by_var[0]) {res.get_node(1).mc_by_var[0]}")


tmp = sum(res.get_node(1).mc_by_nb_vars)
if orig_mc != tmp:
    print(f"error in {args.file} , true mc: {orig_mc}, got (by_nb_features sum[0]) {tmp}")

# print(f"{args.file}, {res.get_node(1).mc}, {orig_mc}")
os.unlink(dDNNF_file)

dimacs = DIMACS.from_file(args.file)

tempfile = args.file + ".tmp"

for i in range(1, dimacs.nb_vars + 1):
    dimacs.cls.append([i])

    dimacs.to_file(tempfile)
    omc = get_mc(tempfile)

    if omc != res.get_node(1).mc_by_var[i]:
        print(f"error in {args.file} for var {i}, true mc: {omc}, got {res.get_node(1).mc_by_var[i]}")

    dimacs.cls.pop()

os.unlink(tempfile + ".nnf")
os.unlink(tempfile)
# print(res.get_node(1).mc)
# print("")
# print(res.get_node(1).mc_by_var)
# res.print_dot()

print(f"done {args.file}")

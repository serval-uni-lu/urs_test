import argparse
import dDNNF

parser = argparse.ArgumentParser()
parser.add_argument("-f", "--file", type=str)

args = parser.parse_args()

dDNNF_file = args.file

res = dDNNF.from_file(dDNNF_file)
res.annotate_mc()
print(res.get_node(1).mc)
print("")
print(res.get_node(1).mc_by_var)
# res.print_dot()

import argparse
import dDNNF

parser = argparse.ArgumentParser()
parser.add_argument("-f", "--file", type=str)

args = parser.parse_args()

dDNNF_file = args.file

res = dDNNF.from_file(dDNNF_file)
res.annotate_mc()

print("sample sizes:")

tmc = res.get_node(1).mc

print("regular chi-squared")
print(tmc * 5)
print("")
print("by var frequencies")

m = res.get_node(1).mc
for i in res.get_node(1).mc_by_var:
    if res.get_node(1).mc_by_var[i] != 0:
        m = min(m, res.get_node(1).mc_by_var[i])

print(tmc * 5 / m)

print("")
print("by nb features frequencies")

m = res.get_node(1).mc
for i in res.get_node(1).mc_by_nb_features:
    if i != 0:
        m = min(m, i)

print(tmc * 5 / m)

print("")
print("monobit (even/uneven features)")

even = 0
for i in range(0, len(res.get_node(1).mc_by_nb_features)):
    if i % 2 == 0:
        even += res.get_node(1).mc_by_nb_features[i]

uneven = tmc - even
m = min(even, uneven)

print(tmc * 5 / m)
# res.print_dot()

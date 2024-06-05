import pandas as pd
import numpy as np
import math

significance_level = 0.01
bench = "unigen3_easy"
batch_size = "b1000"
# tests = ["monobit", "freq_var", "freq_nb_var", "chisquared"]
tests = ["birthday"]

samplers = ["distaware", "kus", "quicksampler", "smarch", "spur", "sts", "cmsgen", "unigen3"]

pad = max(map(lambda x : len(x), samplers))

for test in tests:
    print("& \\multicolumn{2}{c|}{" + test + "}", end = '')

print(" \\\\\nsampler", end = '')

for test in tests:
    print(" & \\#formulae & result", end = '')
print("\\\\\n\\hline")

for s in samplers:
    print(s.ljust(pad, ' '), end = '')
    for test in tests:
        fp = f"{bench}_{test}_{batch_size}_{s}.csv"

        data = pd.read_csv(fp, skipinitialspace = True, index_col = 'file')
        data.dropna(inplace = True)

        nb = len(data)
        if nb != 0:
            pr = 1.0 / (np.sum((1.0 / nb) / data['pvalue']))
        res = " "

        if nb != 0 and pr > significance_level:
            res = "T"
        elif nb == 0:
            res = "-"

        tmp = '{:4}'.format(nb)
        print(f" & {tmp} & {res}", end = '')
    print(" \\\\")


import pandas as pd
import numpy as np
import math

significance_level = 0.01
bench = "r30c90"
batch_size = "b1000"
tests = ["monobit", "freq_var", "freq_nb_var"]
# tests = ["birthday"]

samplers = ["distaware", "kus", "quicksampler", "smarch", "spur", "sts", "cmsgen", "unigen3"]

pad = max(map(lambda x : len(x), samplers))

for test in tests:
    print("& \\multicolumn{2}{c|}{" + test + "}", end = '')

print(" \\\\\nsampler", end = '')

for test in tests:
    print(" & \\#formulae & time", end = '')
print("\\\\\n\\hline")

for s in samplers:
    print(s.ljust(pad, ' '), end = '')
    for test in tests:
        fp = f"csv/{bench}_{test}_{batch_size}_{s}.csv"

        data = pd.read_csv(fp, skipinitialspace = True, index_col = 'file')
        data.dropna(inplace = True)

        nb = len(data)
        if nb != 0:
            tt = np.sum(data['time'])
        res = " "

        if nb != 0:
            res = '{:5.1f}'.format(tt / 3600)
        elif nb == 0:
            res = "-"

        tmp = '{:4}'.format(nb)
        print(f" & {tmp} & {res}", end = '')
    print(" \\\\")


import pandas as pd
import numpy as np
import math

significance_level = 0.01
bench = "unigen3_easy"
batch_size = "b1000"
# tests = ["monobit", "freq_var", "freq_nb_var", "chisquared"]
test = "birthday"

samplers = ["distaware", "kus", "quicksampler", "smarch", "spur", "sts", "cmsgen", "unigen3"]

pad = max(map(lambda x : len(x), samplers))

print("sampler", end = '')

print(" & \\#formulae & result & min & max & average", end = '')
print("\\\\\n\\hline")

for s in samplers:
    print(s.ljust(pad, ' '), end = '')
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

    mi = '{:7.0f}'.format(data['o_repeat'].min())
    ma = '{:7.0f}'.format(data['o_repeat'].max())
    me = '{:7.2f}'.format(data['o_repeat'].mean())
    med = '{:7.0f}'.format(data['o_repeat'].median())

    print(f" & {tmp} & {res} & {mi} & {ma} & {me} & {med} \\\\", end = '')
    print("")


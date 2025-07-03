import pandas as pd
import numpy as np
import math

import scipy.stats as stats

significance_level = 0.01
bench = "omega"
batch_size = "b1000"
tests = ["monobit", "freq_var", "birthday", "freq_nb_var", "chisquared"]
# tests = ["birthday"]


samplers = ["kus", "quicksampler", "smarch", "spur", "sts", "cmsgen", "unigen3"]
sm = {"kus":"KUS", "quicksampler": "QuickSampler", "smarch":"Smarch"
      , "spur": "SPUR", "sts":"STS", "cmsgen": "CMSGen", "unigen3":"UniGen3"}
tm = {"monobit":"Monobit", "birthday": "Birthday", "freq_var":"VF", "freq_nb_var":"SFpC", "chisquared":"GOF"}

pad = max(map(lambda x : len(sm[x]), samplers))

for i in range(0, len(tests)):
    test = tests[i]
    sep = ''
    if i + 1 < len(tests):
        sep = '|'
    print("& \\multicolumn{2}{c" + sep + "}{" + tm[test] + "}", end = '')
print(" \\\\\nsampler", end = '')

for test in tests:
    print(" & |S| & time (h)", end = '')
print("\\\\\n\\hline")

X = []
Y = []
for s in samplers:
    print(sm[s].ljust(pad, ' '), end = '')
    for test in tests:
        fp = f"csv/{bench}_{test}_{batch_size}_c10_{s}.csv"

        data = pd.read_csv(fp, skipinitialspace = True, index_col = 'file')
        # data = data['N']
        data.dropna(inplace = True)

        nb = len(data)
        # if nb != 0:
        #     tt = np.sum(data['time'])
        # res = " "

        # if nb != 0:
        #     res = '{:5.1f}'.format(tt / 3600)
        # elif nb == 0:
        #     res = "-"

        # tmp = '{:5.1f}'.format(np.sum(data['N']) / 1000000)
        tmp = 0
        for i in data['N']:
            tmp += int(i)
        tmp /= 1000000
        # X.append(tt / 3600)
        # Y.append(tmp)
        print(f" & {tmp:5.1f} & {nb:3d}", end = '')

    print(" \\\\")

# print(stats.kendalltau(X, Y))



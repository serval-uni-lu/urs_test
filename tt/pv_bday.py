import pandas as pd
import numpy as np
import math

significance_level = 0.01
bench = "unigen3_easy"
batch_size = "b1000"
# tests = ["monobit", "freq_var", "freq_nb_var", "chisquared"]
test = "birthday"

samplers = ["kus", "quicksampler", "smarch", "spur", "sts", "cmsgen", "unigen3"]
sm = {"kus":"KUS", "quicksampler": "QuickSampler", "smarch":"Smarch"
      , "spur": "SPUR", "sts":"STS", "cmsgen": "CMSGen", "unigen3":"UniGen3"}

pad = max(map(lambda x : len(sm[x]), samplers))

print("sampler", end = '')

print(" & \\#F & p-value & min & max & average & median", end = '')
print("\\\\\n\\hline")

for s in samplers:
    print(sm[s].ljust(pad, ' '), end = '')
    fp = f"csv/{bench}_{test}_{batch_size}_c10_{s}.csv"

    data = pd.read_csv(fp, skipinitialspace = True, index_col = 'file')
    data.dropna(inplace = True)

    nb = len(data)
    if nb != 0:
        pr = 1.0 / (np.sum((1.0 / nb) / data['pvalue']))
        tt = np.sum(data['time'])
    res = " "

    if nb != 0:
        res = f"{pr:5.3f}"
        if pr > significance_level:
            res = "\\textbf{" + res + "}"
    elif nb == 0:
        res = "-"

    stt = "-"
    if nb != 0:
        stt = '{:5.1f}'.format(np.sum(data['time']) / 3600)

    tmp = '{:4}'.format(nb)

    mi = '{:7.0f}'.format(data['o_repeat'].min())
    ma = '{:7.0f}'.format(data['o_repeat'].max())
    me = '{:7.2f}'.format(data['o_repeat'].mean())
    med = '{:7.0f}'.format(data['o_repeat'].median())

    print(f" & {tmp} & {res} & {mi} & {ma} & {me} & {med} \\\\", end = '')
    print("")


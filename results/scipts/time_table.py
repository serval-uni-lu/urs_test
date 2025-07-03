import pandas as pd
import numpy as np
import math

tests = ["freq_var", "birthday", "freq_nb_var", "chisquared"]
modbit = ["modbit_q2", "modbit_q4", "modbit_q8", "modbit_q16", "modbit_q32", "modbit_q64"]
# tests = ["birthday"]

samplers = ["kus", "quicksampler", "smarch", "spur", "sts", "cmsgen", "unigen3", "bddsampler"]
sm = {"kus":"KUS", "quicksampler": "QuickSampler", "smarch":"Smarch"
      , "spur": "SPUR", "sts":"STS", "cmsgen": "CMSGen", "unigen3":"UniGen3"
      , "bddsampler": "BDDSampler" }
tm = {"monobit":"Monobit", "birthday": "Birthday", "freq_var":"VF", "freq_nb_var":"SFpC", "chisquared":"GOF"
        , "modbit_q2" : "Q = 2", "modbit_q4" : "Q = 4", "modbit_q8" : "Q = 8", "modbit_q16" : "Q = 16"
        , "modbit_q32" : "Q = 32", "modbit_q64" : "Q = 64", "modbit_q128" : "Q = 128"}

pad = max(map(lambda x : len(sm[x]), samplers))

def gen_table(bench, batch_size, tests, samplers):
    for i in range(0, len(tests)):
        test = tests[i]
        sep = ''
        if i + 1 < len(tests):
            sep = '|'
        print("& \\multicolumn{2}{c" + sep + "}{" + tm[test] + "}", end = '')

    print(" \\\\\nSampler", end = '')

    for test in tests:
        print(" & \\#F & time (h)", end = '')
    print("\\\\\n\\hline")

    for s in samplers:
        print(sm[s].ljust(pad, ' '), end = '')
        for test in tests:
            fp = f"csv/{bench}_{test}_{batch_size}_c10_{s}.csv"

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

for d in ["omega", "r30c90", "r30c114", "r30c150b1000"]:
    for batch_size in ["b1000", "b2000", "b4000"]:
        print(f"{d} {batch_size} table:\n")
        gen_table(d, batch_size, tests, samplers)
        print("\n\n")


# for d in ["omega", "omega.cl", "r30c90", "r30c114", "r30c150b1000"]:
#     for batch_size in ["b1000", "b2000", "b4000"]:
#         print(f"{d} {batch_size} table:\n")
#         gen_table(d, batch_size, modbit, samplers)
#         print("\n\n")

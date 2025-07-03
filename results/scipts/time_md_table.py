import pandas as pd
import numpy as np
import math

significance_level = 0.01
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


f = open("Scalability Tables.md", "w")

pad = max(map(lambda x : len(sm[x]), samplers))

def gen_table(bench, batch_size, tests, samplers):
    f.write("|")
    for i in range(0, len(tests)):
        test = tests[i]
        sep = ''
        if i + 1 < len(tests):
            sep = '|'
        f.write("|" + tm[test] + "| ")

    f.write("|\n|-|-")
    for i in range(0, len(tests)):
        test = tests[i]
        sep = ''
        if i + 1 < len(tests):
            sep = '|-'
        f.write("|-" + sep)

    f.write("|\n|Sampler")

    for test in tests:
        f.write(" | #F | time (h)")
    f.write("|\n")


    for s in samplers:
        f.write("|" + sm[s].ljust(pad, ' '))
        for test in tests:
            fp = f"csv/{bench}_{test}_b{batch_size}_c10_{s}.csv"

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
            f.write(f" | {tmp} | {res}")
        f.write("|\n")

f.write("## Experimental results\n\n")
f.write("""For each test (and for each
formula), each sampler was called multiple times to generate samples of a specified
batch size.
The
indicated time (in hours) is the accumulated time across all the formulae for which
the test was performed successfully. #F indicates the number of formulae on which
the test was successfully performed (i.e. without timeouts or out-of-memory errors).\n\n""")

for d in ["omega", "r30c90", "r30c114", "r30c150b1000"]:
    f.write(f"### Dataset: {d}\n\n")
    for batch_size in ["1000", "2000", "4000"]:
        f.write(f"Batch size: {batch_size}:\n\n")
        gen_table(d, batch_size, tests, samplers)
        f.write("\n\nModbit test:\n")
        gen_table(d, batch_size, modbit, samplers)
        f.write("\n\n")


# for d in ["omega", "omega.cl", "r30c90", "r30c114", "r30c150b1000"]:
#     f.write(f"### Dataset: {d}\n\n")
#     for batch_size in ["1000", "2000", "4000"]:
#         f.write(f"Batch size: {batch_size}:\n\n")
#         gen_table(d, batch_size, modbit, samplers)
#         f.write("\n\n")

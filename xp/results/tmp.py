from scipy.stats import chi2
from scipy.stats import chisquare
from scipy.stats import kstest
from scipy.stats import uniform
import pandas as pd
import numpy as np
import math

significance_level = 0.01

samplers = ["cmsgen", "quicksampler", "kus", "spur", "smarch", "sts", "unigen3", "distaware"]
tests = ["monobit", "freq_var", "freq_nb_var", "chisquared"]
# tests = ["freq_var"]
benchmarks = ["r30c90", "r30c114", "r30c150b1000", "r50c200"]
#benchmarks = ["unigen3_easy"]

batch_size = 1000

for b in benchmarks:
    # print(b)
    for t in tests:
        #print(f"  {t}")
        for s in samplers:
            csv = f"csv/{b}_{t}_b{batch_size}_{s}.csv"
            data = pd.read_csv(csv, skipinitialspace = True, index_col = 'file')

            data.dropna(inplace = True)
            nb = len(data)

            if nb != 0:
                data['pvalue'].replace(0, 10**(-1))

                pr = 1.0 / ((1.0 / nb) * (np.sum(1.0 / data['pvalue'])))
                nb_rejects = len(data[data.pvalue <= (significance_level / nb)])
                total_time = np.sum(data['time']) / 3600

                print(f"{b}, {t}, {s}, {pr}, {nb_rejects} / {nb}, {total_time}")
            else:
                print(f"{b}, {t}, {s}, -, - / {nb}, {total_time}")

# data.dropna(inplace = True)

# nb = len(data)
# print(f"nb samples: {nb}")
# 
# print("--------------------------------------------------")
# 
# print("Harmonic mean p-value procedure")
# 
# # print(f"T: {len(T)} + F: {len(F)}")
# 
# pr = 1.0 / ((1.0 / nb) * (np.sum(1.0 / data['pvalue'])))
# print(f"pr: {pr}")
# print(f"res: {pr <= significance_level}")

# print("--------------------------------------------------")
# 
# print("Bonferroni procedure")
# 
# nb_rejects = len(data[data.pvalue <= (significance_level / nb)])
# print(f"nb rejects: {nb_rejects}")
# 
# print("--------------------------------------------------")

# print("Holm's step-down procedure")
# 
# pvalues = np.sort(data.pvalue.to_numpy())
# 
# k = 1
# while k <= len(pvalues) and pvalues[k - 1] <= (significance_level / (nb + 1 - k)):
#     k += 1
# 
# print(f"nb rejects: {k - 1}")

# print("--------------------------------------------------")
# 
# print("Kolmogorov-Smirnov test for goodness of fit on p-values")
# 
# pvalues = data.pvalue.to_numpy()
# 
# print(kstest(pvalues, uniform.cdf))
# 
# print("--------------------------------------------------")
# print("total time")

#print(np.sum(data['time']))

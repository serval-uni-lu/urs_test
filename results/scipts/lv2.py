from scipy.stats import chi2
from scipy.stats import chisquare
from scipy.stats import kstest
from scipy.stats import uniform
import pandas as pd
import numpy as np
import math
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-c", "--csv", type=str)
parser.add_argument("-a", type=float, default=0.01)

args = parser.parse_args()

significance_level = args.a
csv = args.csv

data = pd.read_csv(csv, skipinitialspace = True, index_col = 'file')
data.dropna(inplace = True)

nb = len(data)
print(f"nb samples: {nb}")

print("--------------------------------------------------")

print("Harmonic mean p-value procedure")

# print(f"T: {len(T)} + F: {len(F)}")

# pr = 1.0 / ((1.0 / nb) * (np.sum(1.0 / data['pvalue'])))
pr = 1.0 / (np.sum((1.0 / nb) / data['pvalue']))
print(f"pr: {pr}")
print(f"res: {pr <= significance_level}")

print("--------------------------------------------------")

print("Fisher's method")

x2 = -2 * np.sum(np.log(data['pvalue']))

crit = chi2.ppf(1 - 0.01, df = 2 * nb)

print(x2 <= crit)
print(chi2.sf(x2, df = 2 * nb))

print("--------------------------------------------------")

print("Bonferroni procedure")

nb_rejects = len(data[data.pvalue <= (significance_level / nb)])
print(f"nb rejects: {nb_rejects}")

print("--------------------------------------------------")

print("No correction")

nb_rejects = len(data[data.pvalue <= significance_level])
print(f"nb rejects: {nb_rejects}")

print("--------------------------------------------------")

# print("Holm's step-down procedure")
# 
# pvalues = np.sort(data.pvalue.to_numpy())
# 
# k = 1
# while k <= len(pvalues) and pvalues[k - 1] <= (significance_level / (nb + 1 - k)):
#     k += 1
# 
# print(f"nb rejects: {k - 1}")

print("--------------------------------------------------")

print("Kolmogorov-Smirnov test for goodness of fit on p-values")

pvalues = data.pvalue.to_numpy()

print(kstest(pvalues, uniform.cdf))

print("--------------------------------------------------")
print("total time")

print(np.sum(data['time']) / 3600)

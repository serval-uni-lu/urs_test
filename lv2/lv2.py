from scipy.stats import chi2
from scipy.stats import chisquare
import pandas as pd
import numpy as np
import math
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-c", "--csv", type=str)
parser.add_argument("-a", type=float, default=0.05)

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

pr = 1.0 / ((1.0 / nb) * (np.sum(1.0 / data['pvalue'])))
print(f"pr: {pr}")
print(f"res: {pr <= significance_level}")

print("--------------------------------------------------")

print("Bonferroni procedure")

nb_rejects = len(data[data.pvalue <= (significance_level / nb)])
print(f"nb rejects: {nb_rejects}")

print("--------------------------------------------------")

print("Holm's step-down procedure")

pvalues = np.sort(data.pvalue.to_numpy())

k = 1
while k <= len(pvalues) and pvalues[k - 1] <= (significance_level / (nb + 1 - k)):
    k += 1

print(f"nb rejects: {k - 1}")

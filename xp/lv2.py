from scipy.stats import chi2
from scipy.stats import chisquare
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

T = data[data.isUniform == True]['pvalue']
F = data[data.isUniform == False]['pvalue']

nb = len(T) + len(F)
print(f"nb samples: {nb}")
print(f"T: {len(T)} + F: {len(F)}")

pr = 1.0 / ((1.0 / nb) * (np.sum(1.0 / T) + np.sum(1.0 / F)))
print(f"pr: {pr}")
print(f"res: {pr <= significance_level}")

# print("----------------------------------------")
# 
# a = 0.05 / nb
# 
# Fo = np.sum(T < a) + np.sum(F < a)
# To = nb - Fo
# 
# Te = round((1 - significance_level) * nb)
# Fe = round(significance_level * nb)
# # 
# # 
# observed = [To, Fo]
# expected = [Te, Fe]
# # 
# print(f"expected: {Te}, {Fe}, ({Te + Fe})")
# print(f"observed: {To}, {Fo}, ({nb})")
# # 
# X2, pv = chisquare(observed, expected)
# crit = chi2.ppf(1 - significance_level, df = 1)
# print(f"X2 {X2}")
# print(f"crit {crit}")
# print(f"pv {pv}")
# print(f"u {X2 <= crit}")

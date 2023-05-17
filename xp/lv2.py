from scipy.stats import chi2
from scipy.stats import chisquare
import pandas as pd
import math
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-c", "--csv", type=str)
parser.add_argument("-a", type=float, default=0.05)

args = parser.parse_args()

significance_level = args.a
csv = args.csv

data = pd.read_csv(csv, skipinitialspace = True, index_col = 'file')

T = len(data[data.isUniform == True])
F = len(data[data.isUniform == False])
total = T + F

Te = round((1 - significance_level) * total)
Fe = round(significance_level * total)


observed = [T, F]
expected = [Te, Fe]

print(f"expected: {Te}, {Fe}, ({Te + Fe})")
print(f"observed: {T}, {F}, ({total})")

X2, pv = chisquare(observed, expected)
crit = chi2.ppf(1 - significance_level, df = 1)
print(f"X2 {X2}")
print(f"crit {crit}")
print(f"pv {pv}")
print(f"u {X2 <= crit}")

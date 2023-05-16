from scipy.stats import chi2
from scipy.stats import chisquare
import pandas as pd
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-c", "--csv", type=str)
# parser.add_argument("-k", type=int, default=50)
parser.add_argument("-a", type=float, default=0.05)
parser.add_argument("-n", type=int, default=1)
parser.add_argument("-b", "--batch_size", type=int, default=20)
parser.add_argument("-s", "--sampler", type=int, default=0)

args = parser.parse_args()

significance_level = args.a
csv = args.csv

data = pd.read_csv(csv, skipinitialspace = True, index_col = 'file')

print(data)


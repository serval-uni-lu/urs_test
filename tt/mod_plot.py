import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

import math

plt.rcParams['text.usetex'] = True
plt.rcParams["figure.figsize"] = (6.4,4.1)
plt.rcParams['font.size'] = '14'

datasets = {"r30c90": "r30c90", "r30c114": "r30c114", "r30c150b1000": "r30c150b1000", "unigen3_easy": "UniGen"}


mcs = []
labels = []
ticks = []
ntick = 1

for d in datasets:
    fp = f"mod/{d}.csv"

    data = pd.read_csv(fp, skipinitialspace = True, index_col = 'file')
    data.dropna(inplace = True)

    mcs.append(data['mod'])
    labels.append(datasets[d])
    ticks.append(ntick)
    ntick += 1

fig, ax = plt.subplots()
ax.boxplot(mcs)
plt.xticks(ticks, labels)
plt.ylabel("$Q$")

plt.savefig(f"mod_box.pdf", bbox_inches = 'tight')

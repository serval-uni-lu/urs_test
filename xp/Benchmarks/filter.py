import pandas as pd
import os

data = pd.read_csv("unigen3.csv", skipinitialspace = True)

m = data[data.mem < 8000000]
m = m[m.time < (10 * 60)]

for f in m.file:
    print(f)
    os.system(f"cp --parent \"{f}\" tmp")

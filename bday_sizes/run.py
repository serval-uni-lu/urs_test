import math
import numpy as np
import decimal
import pandas as pd

mc =  pd.read_csv("data/mc.csv", skipinitialspace = True, index_col = 'file')

desired = 0.05
factor = 0

if desired < 1.0:
    factor = math.sqrt(-2.01 * math.log(desired))
else:
    factor = math.sqrt(2.01 * desired)


# rng_range = mc['#m'] + 0.01
# rng_range = mc['#m']
# sample_size = np.ceil(factor * np.sqrt(rng_range))

for i in mc.index:
    rng_range = decimal.Decimal(mc['#m'][i])
    sample_size = factor * math.sqrt(rng_range)
    print(f"{i}, {rng_range}, {sample_size}")

import pandas as pd

mc =  pd.read_csv("data/mc.csv", skipinitialspace = True, index_col = 'file')

print(mc)

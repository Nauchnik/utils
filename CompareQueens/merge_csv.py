import pandas as pd
import sys

f1 = sys.argv[1]
f2 = sys.argv[2]

a = pd.read_csv(f1, sep=' ')
b = pd.read_csv(f2, sep=' ')
#b = b.dropna(axis=1)
merged = a.merge(b, on='N')
merged.to_csv("merged.csv", index=False, sep=' ')
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import sys

timelimit = 5000
solvers_names = ['MapleLCMDistChrBt-DL-v3', 'cadical_sr2019']

f_name = sys.argv[1]
print('input file : ' + f_name)
df= pd.read_csv(f_name, delimiter = ' ')
print(df)

df = df[['MapleLCMDistChrBt-DL-v3', 'cadical_sr2019']]
print(df)

myFig = plt.figure();
plt.ylim(0, timelimit)
#bp = df.boxplot()
_, bp = pd.DataFrame.boxplot(df, return_type='both')
#bp = plt.boxplot(df)
myFig.savefig("boxplot.pdf", format="pdf")

print(df.describe())
whiskers = [whiskers.get_ydata() for whiskers in bp['whiskers']]
print(whiskers)

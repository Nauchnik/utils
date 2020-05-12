import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import sys
import glob
import os

timelimit = 5100
solvers_names = ['./cube-glucose-mpi-min2min.sh', './cube-glucose-mpi-min1min.sh', './cube-glucose-mpi-min10sec.sh', './cube-glucose-mpi-nomin.sh']
solvers_short_names = ['cnc-gl-min2m', 'cnc-gl-min1m', 'cnc-gl-min10s', 'cnc-gl-nomin']
solvers_names = [s.replace('./','') for s in solvers_names]

def process_n_stat_file(n_stat_file_name : str):
	n = int(n_stat_file_name.split('_n_')[1].split('.')[0])
	#print('input file : ' + n_stat_file_name)
	df= pd.read_csv(n_stat_file_name, delimiter = ' ')
	#print(df)

	df = df[solvers_names]
	df.columns = solvers_short_names

	medians = dict()
	for s in solvers_short_names:
		medians[s] = df[s].median()
		#print(df[s])
		#print(str(df[s].median()))
	#print(df.describe())

	myFig = plt.figure();
	plt.ylim(0, timelimit)
	_, bp = pd.DataFrame.boxplot(df, return_type='both')
	n_stat_file_name = n_stat_file_name.replace('./','')
	myFig.savefig("boxplot_" + n_stat_file_name.split('.')[0] + ".pdf", format="pdf")

	whiskers = [whiskers.get_ydata() for whiskers in bp['whiskers']]
	#print(whiskers)

	return n, medians

n_stat_mask = sys.argv[1]
print('n_stat_mask : ' + n_stat_mask)
cubes_stat_file_name = sys.argv[2]
print('cubes_stat_file_name : ' + cubes_stat_file_name)

cubes_dict = dict()
df = pd.read_csv(cubes_stat_file_name, delimiter = ' ')
for index, row in df.iterrows():
	cubes_dict[int(row['n'])] = int(row['cubes'])
print('cubes_dict : ')
print(cubes_dict)

os.chdir('./')
n_stat_file_names = []
for fname in glob.glob(n_stat_mask):
	n_stat_file_names.append(fname)

print('n_stat_file_names : ')
print(n_stat_file_names)

total_stat_dict = dict()
for fname in n_stat_file_names:
	n, medians = process_n_stat_file(fname)
	total_stat_dict[n] = medians

with open('total_stat_' + n_stat_mask.replace('*',''), 'w') as ofile:
	ofile.write('n cubes')
	for s in solvers_short_names:
		ofile.write(' median_' + s)
	ofile.write('\n')
	for n in cubes_dict:
		if n not in total_stat_dict:
			continue
		ofile.write('%d %d' % (n, cubes_dict[n]))
		for s in solvers_short_names:
			n_stat = total_stat_dict[n]
			ofile.write(' %.2f' % n_stat[s])
		ofile.write('\n')
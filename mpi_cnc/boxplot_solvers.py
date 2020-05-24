import matplotlib.pyplot as plt
import pandas as pd
import statistics
import numpy as np
import sys
import glob
import os

SOLVER_TIME_LIM = 5000.0
y_limit = 5100
old_solvers_names = ['cube-glucose-mpi-min2min.sh', 'cube-glucose-mpi-min1min.sh', 'cube-glucose-mpi-min10sec.sh', 'cube-glucose-mpi-nomin.sh']
solvers_names = ['./cube-glucose-min2min.sh', './cube-glucose-min1min.sh', './cube-glucose-min10sec.sh', './cube-glucose-nomin.sh']
solvers_short_names = ['gl-min2m', 'gl-min1m', 'gl-min10s', 'gl-nomin']
solvers_short_names_dict = {'./cube-glucose-min2min.sh' : 'gl-min2m', './cube-glucose-min1min.sh' : 'gl-min1m', './cube-glucose-min10sec.sh' : 'gl-min10s', './cube-glucose-nomin.sh' : 'gl-nomin'}

def make_upper_whiskers(df):
	upper_whiskers = dict()
	for col in df.columns:
		q1 = np.percentile(df[col], 25) # q1
		q3 = np.percentile(df[col], 75) # q3
		iqr = q3 - q1
		upper_whisker_bound = q3 + iqr*1.5
		upper_whisker = -1.0
		rev_sort = sorted(df[col], reverse = True)
		for x in rev_sort:
			if x < upper_whisker_bound:
				upper_whisker = x
				break
		upper_whiskers[col] = upper_whisker
	return upper_whiskers
	
def process_n_stat_file(n_stat_file_name : str):
	n = int(n_stat_file_name.split('_n_')[1].split('.')[0])
	print('\n*** n : %d\n' % n)
	#print('input file : ' + n_stat_file_name)
	df= pd.read_csv(n_stat_file_name, delimiter = ' ')
	#print(df)

	#all_columns_names = list(df.columns.values)
	#columns_names = []
	#for col in all_columns_names:
	#	if col n cnfid march-cu-time cubes refuted-leaves 

	#df = df[old_solvers_names]
	#df.columns = solvers_short_names

	del df['n']
	del df['cnfid']
	del df['march-cu-time']
	del df['cubes']
	del df['refuted-leaves']

	df = df.rename(columns={'cube-glucose-mpi-min2min.sh' : 'gl-min2m', 'cube-glucose-mpi-min1min.sh' : 'gl-min1m', 'cube-glucose-mpi-min10sec.sh' : 'gl-min10s', 'cube-glucose-mpi-nomin.sh' : 'gl-nomin'})
	df = df.rename(columns={'march-cu-time_cube-glucose-mpi-min2min.sh' : 'm-gl-min2m', 'march-cu-time_cube-glucose-mpi-min1min.sh' : 'm-gl-min1m', 'march-cu-time_cube-glucose-mpi-min10sec.sh' : 'm-gl-min10s', 'march-cu-time_cube-glucose-mpi-nomin.sh' : 'm-gl-nomin'})
	upper_whiskers = make_upper_whiskers(df)

	#print(df)
	
	myFig = plt.figure();
	plt.ylim(0, y_limit)
	_, bp = pd.DataFrame.boxplot(df, return_type='both')
	n_stat_file_name = n_stat_file_name.replace('./','')
	myFig.savefig("boxplot_" + n_stat_file_name.split('.')[0] + ".pdf", format="pdf")

	#whiskers = [whiskers.get_ydata() for whiskers in bp['whiskers']]
	#print('whiskers :')
	#print(whiskers)
	
	return n, upper_whiskers

def read_samples(samples_file_name : str):
	df_samples = pd.read_csv(samples_file_name, delimiter = ' ')
	samples = dict()
	for index, row in df_samples.iterrows():
		n = int(row['n'])
		t = float(row['time'])
		if t >= SOLVER_TIME_LIM:
			t = SOLVER_TIME_LIM*2 # PAR-2 penalty
		s = solvers_short_names_dict[row['solver']]
		if n not in samples:
			samples[n] = dict()
		if s not in samples[n]:
			samples[n][s] = []
		samples[n][s].append(t)

	samples_mean = dict()
	for n in samples:
		samples_mean[n] = dict()
		for s in samples[n]:
			samples_mean[n][s] = statistics.mean(samples[n][s])
			if n == 2670:
				myFig = plt.figure();
				plt.hist(samples[n][s], bins = 100)
				myFig.savefig('n_' + str(n) + 's_' + s + '_' + samples_file_name.split('.')[0] + ".pdf", format="pdf")
	return samples, samples_mean

n_stat_mask = sys.argv[1]
print('n_stat_mask : ' + n_stat_mask)
cubes_stat_file_name = sys.argv[2]
print('cubes_stat_file_name : ' + cubes_stat_file_name)
samples_file_name = sys.argv[3]
print('samples_file_name : ' + samples_file_name)

cubes_dict = dict()
df = pd.read_csv(cubes_stat_file_name, delimiter = ' ')
for index, row in df.iterrows():
	cubes_dict[int(row['n'])] = int(row['cubes'])
print('cubes_dict : ')
print(cubes_dict)

samples, samples_mean = read_samples(samples_file_name)
#print(samples_mean)
CLUSTER_CORES = 179
CLUSTER_CPU_FRAC = 2.2
samples_unsat_est = dict()
solvers = []
for n in samples_mean:
	samples_unsat_est[n] = dict()
	for s in samples_mean[n]:
		if s not in solvers:
			solvers.append(s)
		samples_unsat_est[n][s] = samples_mean[n][s]	* cubes_dict[n] * CLUSTER_CPU_FRAC / 86400 / CLUSTER_CORES
with open('est_' + samples_file_name, 'w') as est_samples_file:
	est_samples_file.write('n')
	for s in solvers:
		est_samples_file.write(' ' + s)
	est_samples_file.write('\n')
	for n in samples_unsat_est:
		est_samples_file.write('%d' % n)
		#for s in samples_unsat_est[n]:
		for s in solvers:
			est_samples_file.write(' %.2f' % samples_unsat_est[n][s])
		est_samples_file.write('\n')
		
os.chdir('./')
n_stat_file_names = []
for fname in glob.glob(n_stat_mask):
	n_stat_file_names.append(fname)

print('n_stat_file_names : ')
print(n_stat_file_names)

n_solvers_upper_whiskers = dict()
for fname in n_stat_file_names:
	n, upper_whiskers = process_n_stat_file(fname)
	n_solvers_upper_whiskers[n] = upper_whiskers

samples_comb_est = dict()
for n in samples:
	samples_comb_est[n] = dict()
	print('n : %d' % n)
	for s in samples[n]:
		lst_val_less_upper_whisker = [x for x in samples[n][s] if x <= n_solvers_upper_whiskers[n][s]]
		lst_val_greater_upper_whisker = [x for x in samples[n][s] if x > n_solvers_upper_whiskers[n][s]]
		frac_less_upper_whisker = len(lst_val_less_upper_whisker) / len(samples[n][s])
		frac_greater_upper_whisker = len(lst_val_greater_upper_whisker) / len(samples[n][s])
		print('frac_less_upper_whisker : %.2f' % frac_less_upper_whisker)
		print('frac_greater_upper_whisker : %.2f' % frac_greater_upper_whisker)

with open('total_stat_' + n_stat_mask.replace('*',''), 'w') as ofile:
	s_names = []
	for n in cubes_dict:
		for s in n_solvers_upper_whiskers[n]:
			s_names.append(s)
		break
	
	ofile.write('n cubes')
	for s in s_names:
		ofile.write(' up_wh_' + s)
	ofile.write('\n')
	for n in cubes_dict:
		if n not in n_solvers_upper_whiskers:
			continue
		ofile.write('%d %d' % (n, cubes_dict[n]))
		for s in s_names:
			ofile.write(' %.2f' % n_solvers_upper_whiskers[n][s])
		ofile.write('\n')
		
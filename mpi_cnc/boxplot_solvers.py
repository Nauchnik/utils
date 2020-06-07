import matplotlib.pyplot as plt
import pandas as pd
import statistics
import numpy as np
import sys
import glob
import os

CLUSTER_CORES = 179
CLUSTER_CPU_FRAC = 2.2
SOLVER_TIME_LIM = 5000.0
MESSAGE_COST = 0.1 # cost of sending and recieving a message on a cluster
y_limit = 5100
old_solvers_names = ['cube-glucose-mpi-min2min.sh', 'cube-glucose-mpi-min1min.sh', 'cube-glucose-mpi-min10sec.sh', 'cube-glucose-mpi-nomin.sh']
solvers_names = ['./cube-glucose-min2min.sh', './cube-glucose-min1min.sh', './cube-glucose-min10sec.sh', './cube-glucose-nomin.sh']
solvers_short_names = ['gl-min2m', 'gl-min1m', 'gl-min10s', 'gl-nomin']
solvers_short_names_dict = {'./MapleLCMDistChrBt-DL-v3' : 'v3', './kissat' : 'kissat', './cadical' : 'cadical', './cube-glucose-min2min.sh' : 'gl-min2m', './cube-glucose-min1min.sh' : 'gl-min1m', './cube-glucose-min10sec.sh' : 'gl-min10s', './cube-glucose-nomin.sh' : 'gl-nomin'}

def make_medians_upper_whiskers(df):
	medians = dict()
	upper_whiskers = dict()
	for col in df.columns:
		medians[col] = statistics.median(df[col])
		# calculate upper whisker
		q1 = np.percentile(df[col], 25) # q1
		q3 = np.percentile(df[col], 75) # q3
		iqr = q3 - q1
		upper_whisker_bound = q3 + iqr*1.5
		#upper_whisker_bound = np.percentile(df[col], 91, interpolation='higher')
		print('upper_whisker_bound : %.2f' % upper_whisker_bound)
		upper_whisker = -1.0
		rev_sort = sorted(df[col], reverse = True)
		print(rev_sort)
		for x in rev_sort:
			if x <= upper_whisker_bound:
				upper_whisker = x
				break
		upper_whiskers[col] = upper_whisker
		#print('upper_whisker : %.2f' % upper_whisker)
	return medians, upper_whiskers
	
def process_n_stat_file(n_stat_file_name : str):
	n = int(n_stat_file_name.split('_n_')[1].split('.')[0])
	print('\n*** n : %d\n' % n)
	#print('input file : ' + n_stat_file_name)
	df= pd.read_csv(n_stat_file_name, delimiter = ' ')
	#print(df)

	del df['n']
	del df['cnfid']
	del df['march-cu-time']
	del df['cubes']
	del df['refuted-leaves']

	df = df.rename(columns={'cube-glucose-mpi-min2min.sh' : 'gl-min2m', 'cube-glucose-mpi-min1min.sh' : 'gl-min1m', 'cube-glucose-mpi-min10sec.sh' : 'gl-min10s', 'cube-glucose-mpi-nomin.sh' : 'gl-nomin'})
	df = df.rename(columns={'march-cu-time_cube-glucose-mpi-min2min.sh' : 'm-gl-min2m', 'march-cu-time_cube-glucose-mpi-min1min.sh' : 'm-gl-min1m', 'march-cu-time_cube-glucose-mpi-min10sec.sh' : 'm-gl-min10s', 'march-cu-time_cube-glucose-mpi-nomin.sh' : 'm-gl-nomin'})
	# replace -1.0 caused by solving on the minimization phase
	d = {-1.00 : 0.00}
	df = df.replace(d)
	
	medians, upper_whiskers = make_medians_upper_whiskers(df)
	
	myFig = plt.figure();
	plt.ylim(0, y_limit)
	_, bp = pd.DataFrame.boxplot(df, return_type='both')
	n_stat_file_name = n_stat_file_name.replace('./','')
	myFig.savefig("boxplot_" + n_stat_file_name.split('.')[0] + ".pdf", format="pdf")

	#whiskers = [whiskers.get_ydata() for whiskers in bp['whiskers']]
	#print('whiskers :')
	#print(whiskers)
	
	return n, medians, upper_whiskers

def process_sat_samples(sat_samples_files_mask : str, cubes_dict : dict, unsat_samples : dict):
	os.chdir('./')
	n_stat_file_names = []
	for fname in glob.glob(sat_samples_files_mask):
		n_stat_file_names.append(fname)

	print('n_stat_file_names : ')
	print(n_stat_file_names)

	n_solvers_upper_whiskers = dict()
	n_solvers_medians = dict()
	for fname in n_stat_file_names:
		n, n_solvers_medians[n], n_solvers_upper_whiskers[n] = process_n_stat_file(fname)
	
	if len(unsat_samples) > 0:
		samples_comb_est = dict()
		for n in unsat_samples:
			samples_comb_est[n] = dict()
			print('n : %d' % n)
			for s in unsat_samples[n]:
				lst_val_less_median = [x for x in unsat_samples[n][s] if x <= n_solvers_medians[n][s]]
				lst_val_greater_median = [x for x in unsat_samples[n][s] if x > n_solvers_medians[n][s]]
				frac_less_median = len(lst_val_less_median) / len(unsat_samples[n][s])
				frac_greater_median = len(lst_val_greater_median) / len(unsat_samples[n][s])
				print('frac_less_median : %.2f' % frac_less_median)
				print('frac_greater_median : %.2f' % frac_greater_median)

	with open('total_stat_' + sat_samples_files_mask.replace('*',''), 'w') as ofile:
		s_names = []
		for n in cubes_dict:
			for s in n_solvers_upper_whiskers[n]:
				s_names.append(s)
			break
		
		ofile.write('n cubes')
		for s in s_names:
			ofile.write(' m_' + s)
		for s in s_names:
			ofile.write(' up_wh_' + s)
		ofile.write('\n')
		for n in cubes_dict:
			if n not in n_solvers_upper_whiskers:
				continue
			ofile.write('%d %d' % (n, cubes_dict[n]))
			for s in s_names:
				ofile.write(' %.2f' % n_solvers_medians[n][s])
			for s in s_names:
				ofile.write(' %.2f' % n_solvers_upper_whiskers[n][s])
			ofile.write('\n')
		
def read_unsat_samples(unsat_samples_file_name : str):
	df_unsat_samples = pd.read_csv(unsat_samples_file_name, delimiter = ' ')
	samples = dict()
	for index, row in df_unsat_samples.iterrows():
		n = int(row['n'])
		t = float(row['time'])
		if t >= SOLVER_TIME_LIM:
			t = SOLVER_TIME_LIM*2 # PAR-2 penalty
		s = solvers_short_names_dict[row['solver']]
		if n not in unsat_samples:
			unsat_samples[n] = dict()
		if s not in unsat_samples[n]:
			unsat_samples[n][s] = []
		unsat_samples[n][s].append(t)

	unsat_samples_mean = dict()
	for n in unsat_samples:
		unsat_samples_mean[n] = dict()
		for s in unsat_samples[n]:
			unsat_samples_mean[n][s] = statistics.mean(unsat_samples[n][s])
			if n == 2670:
				myFig = plt.figure();
				plt.hist(unsat_samples[n][s], bins = 100)
				myFig.savefig('n_' + str(n) + 's_' + s + '_' + unsat_samples_file_name.split('.')[0] + ".pdf", format="pdf")
	return unsat_samples, unsat_samples_mean

def process_unsat_samples(unsat_samples_file_name : str, cubes_dict : dict ):
	unsat_samples, unsat_samples_mean = read_unsat_samples(unsat_samples_file_name)
	print('unsat_samples_mean : ')
	print(unsat_samples_mean)
	unsat_samples_est = dict()
	solvers = []
	for n in unsat_samples_mean:
		if n not in cubes_dict:
			continue
		unsat_samples_est[n] = dict()
		for s in unsat_samples_mean[n]:
			if s not in solvers:
				solvers.append(s)
			unsat_samples_est[n][s] = (unsat_samples_mean[n][s] + MESSAGE_COST ) * cubes_dict[n] * CLUSTER_CPU_FRAC / 86400 / CLUSTER_CORES 
		with open('est_' + unsat_samples_file_name, 'w') as unsat_samples_est_file:
			unsat_samples_est_file.write('n')
			for s in solvers:
				unsat_samples_est_file.write(' ' + s)
			unsat_samples_est_file.write('\n')
			lst_n = []
			for n in unsat_samples_est:
				lst_n.append(n)
			lst_n.reverse()
			for n in lst_n:
				unsat_samples_est_file.write('%d' % n)
				#for s in samples_unsat_est[n]:
				for s in solvers:
					unsat_samples_est_file.write(' %.5f' % unsat_samples_est[n][s])
				unsat_samples_est_file.write('\n')

# main 

if len(sys.argv) < 3:
    print('Usage: cubes_file [-u=unsat_log] [-s=sat_logs_mask]')
    exit(1)

cubes_stat_file_name = sys.argv[1]
print('cubes_stat_file_name : ' + cubes_stat_file_name)
unsat_samples_file_name = ''
sat_samples_files_mask = ''
for word in sys.argv[2:]:
	if '-u=' in word:
		unsat_samples_file_name = word.split('-u=')[1]
		print('unsat_samples_file_name : ' + unsat_samples_file_name)
	elif '-s=' in word:
		sat_samples_files_mask = word.split('-s=')[1]
		print('sat_samples_files_mask : ' + sat_samples_files_mask)

cubes_dict = dict()
df = pd.read_csv(cubes_stat_file_name, delimiter = ' ')
for index, row in df.iterrows():
	cubes_dict[int(row['n'])] = int(row['cubes'])
print('cubes_dict : ')
print(cubes_dict)

unsat_samples = dict()
if unsat_samples_file_name != '':
	unsat_samples = process_unsat_samples(unsat_samples_file_name, cubes_dict)

if sat_samples_files_mask != '':
	process_sat_samples(sat_samples_files_mask, cubes_dict, unsat_samples)
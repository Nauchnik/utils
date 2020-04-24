import sys
import os
import time
from random import randint
import multiprocessing as mp
import pandas as pd
import find_cnc_n_param

input_vars_number = 512
output_vars_number = 128 # last variables in a CNF
SOLVER_LIMIT_SEC = 5000
TIME_BOARD_FRAC = 0.25
solvers = ['./MapleLCMDistChrBt-DL-v3', './cadical_sr2019', './cube-lingeling-mpi.sh', './cube-glucose-mpi.sh', ]
LING_MIN_LIMIT_SEC = 3600
RANDOM_SAMPLE_SIZE = 20

#template_cnf_name = sys.argv[1]
template_cnf_name = 'md4_40_with_constr_template.cnf'
print('template_cnf_name : ' + template_cnf_name)
stat_name = sys.argv[1]
print('stat_name : ' + stat_name)
df = pd.read_csv(stat_name, delimiter = ' ')
n_zero_time_dict = dict()
for index, row in df.iterrows():
	if int(row['non-refuted-cubes']) < find_cnc_n_param.MAX_NON_REFUTED_CUBES:
		n_zero_time_dict[int(row['n'])] = float(row['time'])
print('n_zero_time_dict : ')
print(n_zero_time_dict)

def clean_garbage():
	print('killing solvers')
	for solver in solvers:
		sys_str = 'pkill -9 ' + solver
		#print(sys_str)
		o = os.popen(sys_str).read()
	print('killing lingeling')
	sys_str = 'pkill -9 ./lingeling'
	#print(sys_str)
	o = os.popen(sys_str).read()
	print('killing march_cu')
	sys_str = 'pkill -9 ./march_cu'
	#print(sys_str)
	o = os.popen(sys_str).read()
	print('killing ilingeling')
	#print(sys_str)
	sys_str = 'pkill -9 ./ilingeling'
	o = os.popen(sys_str).read()
	print('killing iglucose')
	#print(sys_str)
	sys_str = 'pkill -9 ./iglucose'
	o = os.popen(sys_str).read()
	#
	print('removing temporary files')
	sys_str = 'rm ./rand_*'
	#print('start system command : ' + sys_str)
	o = os.popen(sys_str).read()
	sys_str = 'rm ./min_rand_*'
	#print('start system command : ' + sys_str)
	o = os.popen(sys_str).read()
	sys_str = 'rm ./known_sat_cube_*'
	#print('start system command : ' + sys_str)
	o = os.popen(sys_str).read()
	sys_str = 'rm ./cubes_min_*'
	#print('start system command : ' + sys_str)
	o = os.popen(sys_str).read()
	sys_str = 'rm ./id-*'
	o = os.popen(sys_str).read()

def generate_random_values(template_cnf_name : str, cnf_id : int):
	values = dict()
	sys_str = './lingeling --seed=' + str(cnf_id) + ' ' + template_cnf_name
	#print('start system command : ' + sys_str)
	o = os.popen(sys_str).read()
	#print(o)
	s = ''
	lst = o.split('\n')
	for x in lst:
		if x == '':
			continue
		if x[0] == 'v':
			s = x.split('v ')[1]
			int_vals = [int(word) for word in s.split(' ')]
			for iv in int_vals:
				if iv != 0:
					values[abs(iv)] = 1 if iv>0 else 0
	return values

def make_cnf_known_values(template_cnf_name : str, cnf_name : str, known_vars_values : dict):
	cnf_vars_number = -1
	template_cnf_clauses = []
	with open(template_cnf_name, 'r') as ifile:
		lines = ifile.readlines()
		for line in lines:
			if len(line) < 2:
				continue
			if 'p cnf' in line:
				cnf_vars_number = int(line.split(' ')[2])
				#print('cnf_vars_number : %d' % cnf_vars_number)
			else:
			    template_cnf_clauses.append(line)
	#
	cnf_clauses_number = len(template_cnf_clauses) + len(known_vars_values)
	#print('cnf_clauses_number : %d' % cnf_clauses_number)
	with open(cnf_name, 'w') as cnf:
		cnf.write('p cnf ' + str(cnf_vars_number) + ' ' + str(cnf_clauses_number) + '\n')
		for clause in template_cnf_clauses:
			cnf.write(clause)
		for var in known_vars_values:
			s = ''
			if known_vars_values[var] == 0:
				s = '-'
			s += str(var) + ' 0\n'
			cnf.write(s)

def remove_file(file_name):
	sys_str = 'rm ' + file_name
	#print('start system command : ' + sys_str)
	o = os.popen(sys_str).read()

def find_sat_log(o):
	res = False
	lines = o.split('\n')
	for line in lines:
		if len(line) < 12:
			continue
		if 's SATISFIABLE' in line:
			res = True
			break
	return res

def find_n_param(o):
	n = -1
	lines = o.split('\n')
	n_val = -1
	isPerc = False
	for line in reversed(lines):
		if 'n_val :' in line and isPerc == True:
			n = int(line.split(' ')[2])
			break
		if 'new perc rec' in line:
			#print(line)
			isPerc = True
	return n

def get_sat_cube(cubes_name, values_all_vars):
		sat_cubes = []
		with open(cubes_name, 'r') as cubes_file:
				lines = cubes_file.readlines()
				for line in lines:
						lst = line.split(' ')[1:-1] # skip 'a' and '0'
						match_number = 0
						for word in lst:
								val = 0 if word[0] == '-' else 1
								var = abs(int(word))
								if values_all_vars[var] == val:
										match_number += 1
						if match_number == len(lst):
								sat_cubes.append(lst)
		# remove cubes file
		remove_file(cubes_name)
		return sat_cubes

def add_cube(old_cnf_name : str, new_cnf_name : str, cube : list):
		cnf_var_number = 0
		clauses = []
		with open(old_cnf_name, 'r') as cnf_file:
				lines = cnf_file.readlines()
				for line in lines:
						if len(line) < 2 or line[0] == 'c':
								continue
						if line[0] == 'p':
								cnf_var_number = line.split(' ')[2]
						else:
								clauses.append(line)
		clauses_number = len(clauses) + len(cube)
		#print('clauses_number : %d' % clauses_number)
		with open(new_cnf_name, 'w') as cnf_file:
				cnf_file.write('p cnf ' + str(cnf_var_number) + ' ' + str(clauses_number) + '\n')
				for cl in clauses:
						cnf_file.write(cl)
				for c in cube:
						cnf_file.write(c + ' 0\n')

def find_refuted(o):
	#c number of cubes 2635794, including 53902 refuted leaves
	cubes = 0
	refuted = 0
	lines = o.split('\n')
	for line in lines:
		if 'c number of cubes' in line:
			line = line.replace(',','')
			cubes = int(line.split(' ')[4])
			refuted = int(line.split(' ')[6])
	return cubes, refuted
	
def make_cnf_known_sat_cube(n: int, cnf_name : str, values_all_vars : list, original_cnf_march_time_sec : float):
	min_cnf_name = 'min_' + cnf_name
	sys_str = './lingeling -s -T ' + str(LING_MIN_LIMIT_SEC) + ' -o ' + min_cnf_name + ' ' + cnf_name
	#print('start system command : ' + sys_str)
	o = os.popen(sys_str).read()
	isSat = find_sat_log(o)
	if isSat:
		print('*** SAT found by lingeling on the minimization phase')
		print(o)
		exit(1)
	remove_file(cnf_name)
	cubes_name = 'cubes_' + min_cnf_name
	left_board = original_cnf_march_time_sec * (1.0 - TIME_BOARD_FRAC)
	right_board = original_cnf_march_time_sec * (1.0 + TIME_BOARD_FRAC)
	sys_str = './timelimit -T 1 -t ' + str(right_board) + ' ./march_cu ' + min_cnf_name + ' -n ' + str(n) + ' -o ' + cubes_name
	#print('start system command : ' + sys_str)
	march_time_sec = time.time()
	o = os.popen(sys_str).read()
	march_time_sec = time.time() - march_time_sec
	cnf_known_sat_cube_name = ''
	cubes = refuted = 0
	# don't construct a cube_cnf for non matching march_cu time
	if march_time_sec > left_board and march_time_sec < right_board:
		#print('original_cnf_march_time_sec : %f' % original_cnf_march_time_sec)
		#print('left_board : %f' % left_board)
		#print('right_board : %f' % right_board)
		isSat = find_sat_log(o)
		if isSat:
			print('*** SAT found by march_cu')
			print(o)
			exit(1)
		cubes, refuted = find_refuted(o)
		if cubes > 0:
			cnf_known_sat_cube_name = 'known_sat_cube_' + min_cnf_name
			sat_cubes = get_sat_cube(cubes_name, values_all_vars)
			#print('%d sat_cubes :' % len(sat_cubes))
			if len(sat_cubes) == 0:
				exit(1)
			#print(sat_cubes)
			add_cube(min_cnf_name, cnf_known_sat_cube_name, sat_cubes[0])
	remove_file(min_cnf_name)
	return cnf_known_sat_cube_name, march_time_sec, cubes, refuted

def get_solving_time(o):
	#cadical - c total real time since initialization: 
	#maple_v3 - c CPU time              :
	solving_time = 0.0 
	lines = o.split('\n')
	for line in lines:
		if 'c total real time' in line:
			solving_time = float(line.split()[6])
		elif 'c CPU time' in line:
			solving_time = float(line.split()[4])
	return solving_time

def get_solver_march_time(o):
	lines = o.split('\n')
	for line in lines:
		if 'remaining time after cube phase : ' in line:
			s = line.split()[6].replace(',','.')
			t = float(s)
			break
	return float(SOLVER_LIMIT_SEC) - t - float(LING_MIN_LIMIT_SEC)
	
def solve_cnf_id(solvers : list, template_cnf_name : str, cnf_id : int, original_cnf_march_time_sec : float):
	#print('cnf_id : %d' % cnf_id)
	values_all_vars = generate_random_values(template_cnf_name, cnf_id)
	#print('%d values_all_vars : ' % len(values_all_vars))
	#print(values_all_vars)
	output_vars = dict()
	for i in range(len(values_all_vars)-output_vars_number,len(values_all_vars)):
		v = i+1
		output_vars[v] = values_all_vars[v]
	#print('%d output vars :' % len(output_vars))
	cnf_name = 'rand_' + template_cnf_name.replace('./','').split('.')[0] + '_cnfid_' + str(cnf_id) + '.cnf'
	make_cnf_known_values(template_cnf_name, cnf_name, output_vars)
	solvers_times = dict()
	solvers_march_times_sec = dict()
	for solver in solvers_times:
		solvers_times[solver] = -1
		solvers_march_times_sec[solver] = -1
	data = make_cnf_known_sat_cube(n, cnf_name, values_all_vars, original_cnf_march_time_sec)
	#print(data)
	cnf_known_sat_cube_name = data[0]
	march_time_sec = data[1]
	cubes = data[2]
	refuted = data[3]
	if cubes > 0:
		for solver in solvers:
			sys_str = './timelimit -T 1 -t ' + str(SOLVER_LIMIT_SEC) + ' ' + solver + ' ' + cnf_known_sat_cube_name
			# if script-based solver
			if '.sh' in solver:
				sys_str += ' ' + str(cnf_id) + ' ' + str(SOLVER_LIMIT_SEC)
			#print('start system command : ' + sys_str)
			elapsed_time = time.time()
			o = os.popen(sys_str).read()
			elapsed_time = time.time() - elapsed_time
			print(o)
			#solvers_times[solver] = get_solving_time(o)
			solvers_times[solver] = float(elapsed_time)
			# remove temp file for script-based solvers
			if '.sh' in solver:
				solvers_march_times_sec[solver] = get_solver_march_time(o)
				remove_file('./id-' + str(cnf_id) + '-*')
		remove_file(cnf_known_sat_cube_name)
	return cnf_id, march_time_sec, cubes, refuted, solvers_times, solvers_march_times_sec

def collect_result(result):
	global results
	global interrupted_march
	cubes = result[2]
	if cubes > 0:
		results.append(result)
	else:
		interrupted_march += 1

if __name__ == '__main__':
	start_time = time.time()

	print("Total number of processors: ", mp.cpu_count())
	cpu_number = mp.cpu_count()

	print('solvers :')
	print(solvers)
	print('random sample size : %d' % RANDOM_SAMPLE_SIZE)
	print('cpu_number : %d' % cpu_number)

	cnf_ids_prev_runs = []
	last_checked_cnf_id = -1

	clean_garbage()

	for n in n_zero_time_dict:
		print('\n*** n : %d ' % n)
		n_time = time.time()
		pool = mp.Pool(cpu_number)
		results = []
		index_cnf_ids_prev_runs = 0
		interrupted_march = 0
		while len(results) < RANDOM_SAMPLE_SIZE:
			if len(cnf_ids_prev_runs) == 0 or index_cnf_ids_prev_runs == len(cnf_ids_prev_runs):
				# if first run, i.e. there is no history of previously used ids
				# or there are no more previous ids to check
				cnf_id = last_checked_cnf_id + 1
				last_checked_cnf_id = cnf_id
			elif len(cnf_ids_prev_runs) > 0 and index_cnf_ids_prev_runs < len(cnf_ids_prev_runs):
				cnf_id = cnf_ids_prev_runs[index_cnf_ids_prev_runs]
				index_cnf_ids_prev_runs += 1
			pool.apply_async(solve_cnf_id, args=(solvers, template_cnf_name, cnf_id, n_zero_time_dict[n]), callback=collect_result)
			while len(pool._cache) >= cpu_number and len(results) < RANDOM_SAMPLE_SIZE: # wait until any cpu is free
				time.sleep(2)
			if len(results) >= RANDOM_SAMPLE_SIZE:
				print('terminating pool')
				pool.terminate()
				break
		pool.close()
		pool.join()
		clean_garbage()
		if len(results) > RANDOM_SAMPLE_SIZE:
			results = results[:RANDOM_SAMPLE_SIZE]
		print('last_checked_cnf_id : %d' % last_checked_cnf_id)
		# add cnf ids to use them in next runs
		for res in results:
			if res[0] not in cnf_ids_prev_runs:
				cnf_ids_prev_runs.append(res[0])
		print('cnf_ids_prev_runs len : %d' % len(cnf_ids_prev_runs))
		print('interrupted_march : %d' % interrupted_march)
		print('results len : %d' % len(results))
		for r in results:
			print(r)
		# write header to an output file
		csv_file_name = 'stat_' + template_cnf_name.replace('./','').split('.')[0] + '_n_' + str(n) + '.csv'
		with open(csv_file_name, 'w') as csv_file:
			csv_file.write('cnf_id march_cu_time total_cubes refuted_cubes')
			# solvers times
			for solver in solvers:
				csv_file.write(' ' + solver.replace('./',''))
			# solvers march times
			for solver in solvers:
				csv_file.write(' march_cu_time_' + solver.replace('./',''))
			csv_file.write('\n')
			for result in results:
				csv_file.write('%d %.2f %d %d' % (result[0], result[1], result[2], result[3]))
				for solver in solvers:
					csv_file.write(' %.2f' % result[4][solver])
				for solver in solvers:
					csv_file.write(' %.2f' % result[5][solver])
				csv_file.write('\n')
		n_time = time.time() - n_time
		print('n time : %.2f' % n_time)
	
	elapsed_time = time.time() - start_time
	print('elapsed_time : ' + str(elapsed_time))
	
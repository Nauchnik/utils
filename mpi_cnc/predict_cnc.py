import sys
import glob
import os
import time
import multiprocessing as mp
import pandas as pd
import logging
import find_cnc_n_param as fcnp

input_vars_number = 512
output_vars_number = 128 # last variable number in a CNF
SOLVER_TIME_LIMIT = 5000
CUBES_BOARD_FRAC = 0.25
MARCH_BOARD_FRAC = 0.25
#solvers = ['./MapleLCMDistChrBt-DL-v3', './cadical_sr2019', './cube-lingeling-mpi.sh', './cube-glucose-mpi.sh']
#solvers = ['./cube-glucose-mpi.sh']
solvers = ['./cube-glucose-mpi-min1min.sh', './cube-glucose-mpi-min10sec.sh', './cube-glucose-mpi-nomin.sh']
sh_solvers = [s for s in solvers if '.sh' in s]
LING_MIN_TIME_LIMIT = 120
RANDOM_SAMPLE_SIZE = 30

results = dict()
interrupted = 0
non_match_cubes = 0

def clean_garbage():
	logging.info('killing processes')
	logging.debug('killing timelimit.sh')
	sys_str = 'killall -9 ./timelimit.sh'
	o = os.popen(sys_str).read()
	logging.debug('killing solvers')
	for solver in solvers:
		sys_str = 'killall -9 ' + solver
		o = os.popen(sys_str).read()
	logging.debug('killing lingeling')
	sys_str = 'killall -9 ./lingeling'
	o = os.popen(sys_str).read()
	logging.debug('killing march_cu')
	sys_str = 'killall -9 ./march_cu'
	o = os.popen(sys_str).read()
	logging.debug('killing ilingeling')
	sys_str = 'killall -9 ./ilingeling'
	o = os.popen(sys_str).read()
	logging.debug('killing iglucose')
	sys_str = 'killall -9 ./iglucose'
	o = os.popen(sys_str).read()
	# kill solvers one more time to be sure
	logging.debug('killing solvers')
	for solver in solvers:
		sys_str = 'killall -9 ' + solver
		o = os.popen(sys_str).read()
	logging.info('removing temporary files')
	sys_str = 'rm ./rand_*'
	o = os.popen(sys_str).read()
	sys_str = 'rm ./min_rand_*'
	o = os.popen(sys_str).read()
	sys_str = 'rm ./known_sat_cube_*'
	o = os.popen(sys_str).read()
	sys_str = 'rm ./cubes_min_*'
	o = os.popen(sys_str).read()
	sys_str = 'rm ./id-*'
	o = os.popen(sys_str).read()

def generate_random_values(template_cnf_name : str, cnf_id : int):
	values = dict()
	sys_str = './lingeling --seed=' + str(cnf_id) + ' ' + template_cnf_name
	o = os.popen(sys_str).read()
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
	
def make_cnf_known_sat_cube(n : int, cnf_name : str, values_all_vars : list, original_data_val : tuple):
	min_cnf_name = 'min_' + cnf_name
	sys_str = './lingeling -s -T ' + str(LING_MIN_TIME_LIMIT) + ' -o ' + min_cnf_name + ' ' + cnf_name
	#print('start system command : ' + sys_str)
	o = os.popen(sys_str).read()
	isSat = find_sat_log(o)
	if isSat:
		print('*** SAT found by lingeling on the minimization phase')
		print(o)
		exit(1)
	remove_file(cnf_name)
	cubes_name = 'cubes_' + min_cnf_name
	original_cnf_march_time = float(original_data_val[0])
	original_cnf_cubes = float(original_data_val[1])
	left_board_cubes = int(original_cnf_cubes * (1.0 - CUBES_BOARD_FRAC))
	right_board_cubes = int(original_cnf_cubes * (1.0 + CUBES_BOARD_FRAC))
	left_board_march_time = float(original_cnf_march_time * (1.0 - MARCH_BOARD_FRAC))
	right_board_march_time = float(original_cnf_march_time * (1.0 + MARCH_BOARD_FRAC))
	sys_str = './timelimit -T 1 -t ' + str(int(right_board_march_time)) + ' ./march_cu ' + min_cnf_name + ' -n ' + str(n) + ' -o ' + cubes_name
	#print('start system command : ' + sys_str)
	march_time = time.time()
	o = os.popen(sys_str).read()
	march_time = time.time() - march_time
	cubes, refuted_leaves = fcnp.parse_march_log(o)
	cnf_known_sat_cube_name = ''
	# don't construct a cube_cnf for non matching number of cubes
	if cubes >= left_board_cubes and cubes <= right_board_cubes:
		#print('original_cnf_march_time : %f' % original_cnf_march_time)
		#print('left_board : %f' % left_board)
		#print('right_board : %f' % right_board)
		isSat = find_sat_log(o)
		if isSat:
			print('*** SAT found by march_cu')
			print(o)
			exit(1)
		cnf_known_sat_cube_name = 'known_sat_cube_' + min_cnf_name
		sat_cubes = get_sat_cube(cubes_name, values_all_vars)
		#print('%d sat_cubes :' % len(sat_cubes))
		if len(sat_cubes) == 0:
			print('*** sat_cubes is empty')
			exit(1)
		#print(sat_cubes)
		add_cube(min_cnf_name, cnf_known_sat_cube_name, sat_cubes[0])
	else:
		if cubes > 0 and march_time >= left_board_march_time and march_time <= right_board_march_time:
			global non_match_cubes
			non_match_cubes += 1
		cubes = 0
	
	remove_file(min_cnf_name)
	return cnf_known_sat_cube_name, march_time, cubes, refuted_leaves

def get_cnfids_from_files( par : str ):
	if '=' not in par:
		logging.error('wrong input parameter : ' + par)
		exit(1)
	logging.info('*** loading cnf ids from the previous runs')
	cnfids_from_files = dict()
	load_mode = 0
	if '-contr-point=' in par:
		load_mode = 1
	elif '-load-cnfids=' in par:
		load_mode = 2
	logging.info('load_mode : %d' % load_mode)
	prev_stat_name_mask = par.split('=')[1]
	#stat_name_mask = 'stat_md4_40_with_constr_template_n_*'	
	if prev_stat_name_mask[-1] != '*':
		prev_stat_name_mask += '*'
	logging.info('prev_stat_name_mask : ' + str(prev_stat_name_mask))
	os.chdir('./')
	prev_stat_name_file_names = []
	for file_name in glob.glob(prev_stat_name_mask):
		prev_stat_name_file_names.append(file_name)
	logging.info('stat_name_file_names :')
	logging.info(prev_stat_name_file_names)
	for fname in prev_stat_name_file_names:
		n = int(fname.split('.csv')[0].split('_')[-1])
		cnfids_from_files[n] = []
		df = pd.read_csv(fname, delimiter = ' ')
		for index, row in df.iterrows():
			cnfid = int(row['cnfid'])
			cnfids_from_files[n].append(cnfid)
	logging.info('cnfids_from_files :')
	logging.info(cnfids_from_files)
	return cnfids_from_files, load_mode

def get_sh_solver_min_time(sh_solver):
	min_time = 0.0
	with open(sh_solver, 'r') as ifile:
		lines = ifile.readlines()
		for line in lines:
			if 'LINGTIMELIM=' in line:
				min_time = float(line.split('LINGTIMELIM=')[1])
	return min_time
	
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

def get_solver_march_time(o, min_time):
	march_time = -1.0
	lines = o.split('\n')
	for line in lines:
		if 'remaining time after cube phase : ' in line:
			s = line.split()[6].replace(',','.')
			#logging.info(line)
			#logging.info('min_time : %f' % min_time)
			march_time = float(SOLVER_TIME_LIMIT) - float(s) - float(min_time)
			#logging.info('march_time : %f' % march_time)
			break
	return march_time
	
def solve_cnf_id(n : int, solvers : list, template_cnf_name : str, cnf_id : int, original_data_val : tuple):
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
	solvers_march_times = dict()
	for solver in solvers:
		solvers_times[solver] = -1
		solvers_march_times[solver] = -1
	data = make_cnf_known_sat_cube(n, cnf_name, values_all_vars, original_data_val)
	#print(data)
	cnf_known_sat_cube_name = data[0]
	march_time = data[1]
	cubes = data[2]
	refuted_leaves = data[3]
	if cubes > 0 and cnf_known_sat_cube_name != '':
		for solver in solvers:
			sys_str = './timelimit -T 1 -t ' + str(SOLVER_TIME_LIMIT) + ' ' + solver + ' ' + cnf_known_sat_cube_name
			# if script-based solver
			if '.sh' in solver:
				sys_str += ' ' + str(cnf_id) + ' ' + str(SOLVER_TIME_LIMIT)
			#print('start system command : ' + sys_str)
			elapsed_time = time.time()
			o = os.popen(sys_str).read()
			elapsed_time = time.time() - elapsed_time
			#print(o)
			#solvers_times[solver] = get_solving_time(o)
			solvers_times[solver] = float(elapsed_time)
			# remove temp file for script-based solvers
			if '.sh' in solver:
				solvers_march_times[solver] = get_solver_march_time(o, min_time_sh_solvers[solver])
				remove_file('./id-' + str(cnf_id) + '-*')
		remove_file(cnf_known_sat_cube_name)
	return n, cnf_id, march_time, cubes, refuted_leaves, solvers_times, solvers_march_times

def collect_result(result):
	global results
	global interrupted
	n = result[0]
	cubes = result[3]
	solvers_times = result[5]
	isSolverTimesOk = True
	for solver in solvers_times:
		if solvers_times[solver] < 0:
			isSolverTimesOk = False
			break
	if cubes > 0 and isSolverTimesOk:
		if result in results[n]:
			logging.error('result is already in the list')
			logging.error(result)
			logging.error(results[n])
			# make the size RANDOM_SAMPLE_SIZE to exit the program
			while len(results[n]) < RANDOM_SAMPLE_SIZE:
				results[n].append(results[0])
		results[n].append(result)
		logging.info('got %d results' % len(results[n]))
		logging.info(result)
	else:
		interrupted += 1
	
if __name__ == '__main__':
	#template_cnf_name = 'md4_40_with_constr_template.cnf'
	if len(sys.argv) < 3:
	    print('Usage: script template_cnf_file stat_file [-contr-point=prev_stat_name_mask | -load-cnfids=prev_stat_name_mask])')
	template_cnf_name = sys.argv[1]
	stat_name = sys.argv[2]
	log_name = 'predict_' + stat_name.replace('./','').replace('.','') + '.log'
	sys_str = 'cp ./' + log_name + ' ./copy_' + log_name
	o = os.popen(sys_str).read()
	logging.basicConfig(filename=log_name, filemode = 'w', level=logging.INFO)
	logging.info('template_cnf_name : ' + template_cnf_name)
	logging.info('stat_name : ' + stat_name)

	min_time_sh_solvers = dict()
	for s in sh_solvers:
		min_time_sh_solvers[s] = get_sh_solver_min_time(s)
	logging.info('min_time_sh_solvers:')
	logging.info(min_time_sh_solvers)
	
	cnf_ids_prev_runs = []
	n_prev_runs = []
	cnfids_from_files = dict()
	last_checked_cnf_id = -1
	load_mode = 0
	if len(sys.argv) >= 4:
		cnfids_from_files, load_mode = get_cnfids_from_files(sys.argv[3])
		if load_mode == 1:
			for n in cnfids_from_files:
				n_prev_runs.append(n)
				for cnfid in cnfids_from_files[n]:
					if cnfid not in cnf_ids_prev_runs:
						cnf_ids_prev_runs.append(cnfid)
			cnf_ids_prev_runs = sorted(cnf_ids_prev_runs, reverse=True) 
			last_checked_cnf_id = cnf_ids_prev_runs[0]
			logging.info('cnf_ids_prev_runs size is %d :' % len(cnf_ids_prev_runs))
			logging.info(cnf_ids_prev_runs)
			logging.info('last_checked_cnf_id : %d' % last_checked_cnf_id)
			logging.info('n_prev_runs :')
			logging.info(n_prev_runs)
		
	df = pd.read_csv(stat_name, delimiter = ' ')
	original_data_dict = dict()
	for index, row in df.iterrows():
		n = int(row['n'])
		cubes = int(row['cubes'])
		m_t = float(row['march-cu-time'])
		if n not in n_prev_runs and cubes <= fcnp.MAX_CUBES and m_t >= fcnp.MIN_MARCH_TIME and m_t <= fcnp.MAX_MARCH_TIME:
			original_data_dict[n] = (m_t,cubes)
	logging.info('original_data_dict : ')
	logging.info(original_data_dict)
	
	start_time = time.time()
	logging.info("Total number of processors: %d", mp.cpu_count())
	cpu_number = mp.cpu_count()
	if cpu_number > 16:
		cpu_number = int(cpu_number/2)
	logging.info('cpu_number : %d' % cpu_number)
	logging.info('solvers :')
	logging.info(solvers)
	logging.info('sh_solvers :')
	logging.info(sh_solvers)
	logging.info('random sample size : %d' % RANDOM_SAMPLE_SIZE)

	clean_garbage()
	pool = mp.Pool(cpu_number)
	
	for n in original_data_dict:
		logging.info(' ')
		logging.info('***')
		logging.info('n : %d ' % n)
		n_time = time.time()
		index_cnf_ids_prev_runs = 0
		interrupted = 0
		non_match_cubes = 0
		results[n] = []
		if load_mode == 2:
			cnf_ids_prev_runs = sorted(cnfids_from_files[n])
			# fill list with additional non-matching values to obtain only results of the same ids
			# thus all cores will be used all the time
			first_non_match_val = -1
			x = cnf_ids_prev_runs[0]
			while first_non_match_val < 0:
				x += 1
				if x not in cnf_ids_prev_runs:
					first_non_match_val = x
			for i in range(100000):
				cnf_ids_prev_runs.append(first_non_match_val)
			logging.info('cnf_ids_prev_runs')
			logging.info(cnf_ids_prev_runs)
			logging.info('first_non_match_val : %d' % first_non_match_val)
		while len(results[n]) < RANDOM_SAMPLE_SIZE:
			# if first run, i.e. there is no history of previously used ids
			# or there are no more previous ids to check
			# else get previous cnf ids
			if len(cnf_ids_prev_runs) == 0 or index_cnf_ids_prev_runs == len(cnf_ids_prev_runs):
				if load_mode == 2:
					logging.error('last_checked_cnf_id is increased in load_mode 2')
					exit(-1)
				last_checked_cnf_id += 1
				cnf_id = last_checked_cnf_id
			elif len(cnf_ids_prev_runs) > 0 and index_cnf_ids_prev_runs < len(cnf_ids_prev_runs):
				cnf_id = cnf_ids_prev_runs[index_cnf_ids_prev_runs]
				index_cnf_ids_prev_runs += 1
			else:
				logging.error('wrong case in main loop')
				exit(-1)
			pool.apply_async(solve_cnf_id, args=(n, solvers, template_cnf_name, cnf_id, original_data_dict[n]), callback=collect_result)
			while len(pool._cache) >= cpu_number and len(results[n]) < RANDOM_SAMPLE_SIZE: # wait until any cpu is free
				time.sleep(2)
			if len(results[n]) >= RANDOM_SAMPLE_SIZE:
				for s in solvers: # kill processes for every solver because they are run in a loop
					clean_garbage()
					time.sleep(2) # wait for processes' termination
				break
		
		if len(results[n]) > RANDOM_SAMPLE_SIZE:
			results[n] = results[n][:RANDOM_SAMPLE_SIZE]
		# add cnf ids to use them in next runs
		if load_mode == 2:
			cnf_ids_prev_runs = []
			last_checked_cnf_id = -1
		else:
			for res in results[n]:
				cnfid = res[1]
				if cnfid not in cnf_ids_prev_runs:
					cnf_ids_prev_runs.append(cnfid)
			# sort ids in descending order - to check the last ids first
			cnf_ids_prev_runs = sorted(cnf_ids_prev_runs, reverse=True) 
			last_checked_cnf_id = cnf_ids_prev_runs[0]
		logging.info('cnf_ids_prev_runs len : %d' % len(cnf_ids_prev_runs))
		logging.info('cnf_ids_prev_runs : ')
		logging.info(cnf_ids_prev_runs)
		logging.info('last_checked_cnf_id : %d' % last_checked_cnf_id)
		logging.info('interrupted : %d' % interrupted)
		logging.info('non_match_cubes : %d' % non_match_cubes)
		logging.info('results[n] len : %d' % len(results[n]))
		for res in results[n]:
			logging.info(res)
			if res[0] != n:
				logging.error('res[0] != n')
				exit(-1)
		# write header to an output file
		csv_file_name = 'stat_' + template_cnf_name.replace('./','').split('.')[0] + '_n_' + str(n) + '.csv'
		with open(csv_file_name, 'w') as csv_file:
			csv_file.write('n cnfid march-cu-time cubes refuted-leaves')
			# solvers times
			for solver in solvers:
				csv_file.write(' ' + solver.replace('./',''))
			# solvers march times
			for solver in sh_solvers:
				csv_file.write(' march-cu-time_' + solver.replace('./',''))
			csv_file.write('\n')
			for res in results[n]:
				csv_file.write('%d %d %.2f %d %d' % (res[0], res[1], res[2], res[3], res[4]))
				for solver in solvers:
					csv_file.write(' %.2f' % res[5][solver])
				for solver in sh_solvers:
					csv_file.write(' %.2f' % res[6][solver])
				csv_file.write('\n')
		n_time = time.time() - n_time
		logging.info('n=%d time : %.2f' % (n,n_time))

	pool.close()
	pool.join()
	
	elapsed_time = time.time() - start_time
	logging.info('total elapsed_time : ' + str(elapsed_time))
	
import sys
import os
import time
import multiprocessing as mp
import random
import collections
import logging
import predict_cnc as p_c

MIN_REFUTED_LEAVES = 1000
MIN_CUBES = 10000
MAX_CUBES = 1000000
MAX_MARCH_TIME = 86400.0
RANDOM_SAMPLE_SIZE = 1000
SOLVER_TIME_LIMIT = 5000
cnf_name = ''
stat_name = ''
start_time = 0.0

def kill_unuseful_processes():
	sys_str = 'killall -9 ./march_cu'
	o = os.popen(sys_str).read()
	sys_str = 'killall -9 ./timelimit'
	o = os.popen(sys_str).read()
	
def remove_file(file_name):
	sys_str = 'rm ' + file_name
	o = os.popen(sys_str).read()

def get_free_vars(cnf_name):
	free_vars = []
	with open(cnf_name) as cnf:
		lines = cnf.readlines()
		for line in lines:
			if line[0] == 'p' or line[0] == 'c':
				continue
			lst = line.split(' ')
			for x in lst:
				if x == ' ' or x == '':
					continue
				var = abs(int(x))
				if var != 0 and var not in free_vars:
					free_vars.append(var)
	return free_vars

def parse_march_log(o):
	cubes = -1
	refuted_leaves = -1
	lines = o.split('\n')
	for line in lines:
		if 'c number of cubes' in line:
			cubes = int(line.split('c number of cubes ')[1].split(',')[0])
			refuted_leaves = int(line.split(' refuted leaves')[0].split(' ')[-1])
	return cubes, refuted_leaves

def get_random_cubes(cubes_name):
	cubes = []
	with open(cubes_name, 'r') as cubes_file:
		lines = cubes_file.readlines()
		for line in lines:
			lst = line.split(' ')[1:-1] # skip 'a' and '0'
			cubes.append(lst)
	if len(cubes) < RANDOM_SAMPLE_SIZE:
		logging.error('too few cubes : %d' % len(cubes))
		exit(1)
	random_cubes = random.sample(cubes, RANDOM_SAMPLE_SIZE)
	return random_cubes
	
def process_n(n : int, cnf_name : str):
	print('n : %d' % n)
	start_t = time.time()
	cubes_name = './cubes_n_' + str(n) + '_' + cnf_name.replace('./','')
	system_str = './timelimit -T 1 -t ' + str(int(MAX_MARCH_TIME)) +  ' ./march_cu ' + cnf_name + \
	' -n ' + str(n) + ' -o ' + cubes_name
	#print('system_str : ' + system_str)
	o = os.popen(system_str).read()
	t = time.time() - start_t
	cubes_num = -1
	refuted_leaves = -1
	march_time = -1.0
	cubes_num, refuted_leaves = parse_march_log(o)
	march_time = float(t)
	#print('elapsed_time : %.2f' % elapsed_time)
	
	return n, cubes_num, refuted_leaves, march_time, cubes_name

def collect_n_result(res):
	global random_cubes_n
	global is_exit
	global is_unsat_sample_solving
	n = res[0]
	cubes_num = res[1]
	refuted_leaves = res[2]
	march_time = res[3]
	cubes_name = res[4]
	if cubes_num >= MIN_CUBES and cubes_num <= MAX_CUBES and refuted_leaves >= MIN_REFUTED_LEAVES:
		logging.info(res)
		ofile = open(stat_name,'a')
		ofile.write('%d %d %d %.2f\n' % (n, cubes_num, refuted_leaves, march_time))
		ofile.close()
		if is_unsat_sample_solving:
			random_cubes = []
			random_cubes = get_random_cubes(cubes_name)
			random_cubes_n[n] = random_cubes
	elif cubes_num > MAX_CUBES or march_time > MAX_MARCH_TIME:
		is_exit = True
		logging.info('is_exit : ' + str(is_exit))
	remove_file(cubes_name)
	
def process_cube(cnf_name : str, n : int, cube : list, cube_index : int):
	known_cube_cnf_name = './sample_cnf_n_' + str(n) + '_cube_' + str(cube_index) + '.cnf'
	p_c.add_cube(cnf_name, known_cube_cnf_name, cube)
	solvers_times = dict()

	isSat = False
	for solver in p_c.solvers:
		if '.sh' in solver:
			sys_str = solver + ' ' + known_cube_cnf_name + ' ' + str(cube_index) + ' ' + str(SOLVER_TIME_LIMIT)
		else:
			sys_str = './timelimit -T 1 -t ' + str(SOLVER_TIME_LIMIT) + ' ' + solver + ' ' + known_cube_cnf_name
		#print('system command : ' + sys_str)
		t = time.time()
		o = os.popen(sys_str).read()
		t = time.time() - t
		solvers_times[solver] = float(t)
		isSat = p_c.find_sat_log(o)
		if isSat:
			sat_name = known_cube_cnf_name.replace('.cnf','')
			sat_name = sat_name.replace('./','')
			with open('!sat_' + sat_name, 'w') as ofile:
				ofile.write('*** SAT found\n')
				ofile.write(o)
	remove_file(known_cube_cnf_name)
	# remove files from solver's script
	remove_file('./id-' + str(cube_index) + '-*')
	return n, solvers_times, isSat
	
def collect_cube_result(res):
	global solvers_results
	n = res[0]
	solvers_times = res[1]
	isSat = res[2]
	solvers_results[n].append(solvers_times)
	logging.info('n : %d, got %d results' % (n, len(solvers_results[n])))
	if isSat:
		logging('*** SAT found')
		exit(1)
		elapsed_time = time.time() - start_time
		logging.info('elapsed_time : ' + str(elapsed_time))
	
if __name__ == '__main__':
	cpu_number = mp.cpu_count()
	if cpu_number > 16:
		cpu_number = int(cpu_number/2)
	
	pool = mp.Pool(cpu_number)
	is_exit = False

	if len(sys.argv) < 2:
		print('Usage : prog cnf-name')
		exit(1)
	cnf_name = sys.argv[1]

	log_name = './find_n_' + cnf_name.replace('./','').replace('.','') + '.log'
	print('log_name : ' + log_name)
	logging.basicConfig(filename=log_name, filemode = 'w', level=logging.INFO)

	logging.info('cnf : ' + cnf_name)
	logging.info("total number of processors: %d" % mp.cpu_count())
	logging.info('cpu_number : %d' % cpu_number)

	is_unsat_sample_solving = True
	if len(sys.argv) > 2:
		if sys.argv[2] == '--nosample':
			is_unsat_sample_solving = False
	logging.info('is_unsat_sample_solving : ' + str(is_unsat_sample_solving))
		
	start_time = time.time()
		
	# count free variables
	free_vars = get_free_vars(cnf_name)
	logging.info('free vars : %d' % len(free_vars))
	n = len(free_vars)
	while n % 10 != 0:
		n -= 1
	logging.info('start n : %d ' % n)

	# prepare an output file
	stat_name = 'stat_' + cnf_name
	stat_name = stat_name.replace('.','')
	stat_name = stat_name.replace('/','')
	stat_file = open(stat_name,'w')
	stat_file.write('n cubes refuted-leaves march-cu-time\n')
	stat_file.close()

	random_cubes_n = dict()
	# find required n and their cubes numbers
	while not is_exit:
		pool.apply_async(process_n, args=(n, cnf_name), callback=collect_n_result)
		while len(pool._cache) >= cpu_number: # wait until any cpu is free
			time.sleep(2)
		n -= 10
		if is_exit or n <= 0:
			#print('terminating pool')
			#pool.terminate()
			logging.info('killing unuseful processes')
			kill_unuseful_processes()
			time.sleep(2) # wait for processes' termination
			break
	
	elapsed_time = time.time() - start_time
	logging.info('elapsed_time : ' + str(elapsed_time))
	logging.info('random_cubes_n : ')
	#print(random_cubes_n)
	
	if is_unsat_sample_solving:
		# sort dict by n in descending order
		sorted_random_cubes_n = collections.OrderedDict(sorted(random_cubes_n.items()))
		logging.info('sorted_random_cubes_n : ')
		logging.info(sorted_random_cubes_n)
		# for evary n solve cube-problems from the random sample
		logging.info('')
		logging.info('processing random samples')
		logging.info('')

		solvers_results = dict()
		for n, random_cubes in sorted_random_cubes_n.items():
			logging.info('*** n : %d' % n)
			logging.info('random_cubes size : %d' % len(random_cubes))
			solvers_results[n] = []
			cube_index = 0
			for cube in random_cubes:
				pool.apply_async(process_cube, args=(cnf_name, n, cube, cube_index), callback=collect_cube_result)
				cube_index += 1
			# wait for all results
			while len(solvers_results[n]) < RANDOM_SAMPLE_SIZE:
				time.sleep(5)
			logging.info('solvers_results[n] len : %d' % len(solvers_results[n]))
			logging.info(solvers_results[n])
	
	pool.close()
	pool.join()
	
	if is_unsat_sample_solving:
		# prepare file for results
		sample_name = 'sample_results_' + cnf_name
		sample_name = sample_name.replace('.','')
		sample_name = sample_name.replace('/','')
		sample_name += '.csv'
		sample_file = open(sample_name, 'w')
		sample_file.write('n solver time\n')
		for n in solvers_results:
			for solvers_times in solvers_results[n]:
				for s, t in solvers_times.items():
					sample_file.write('%d %s %.2f\n' % (n, s, t))
		sample_file.close()

	elapsed_time = time.time() - start_time
	logging.info('elapsed_time : ' + str(elapsed_time))

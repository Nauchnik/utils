import sys
import os
import time
import multiprocessing as mp
import random
import collections
import logging
import predict_cnc as p_c

MIN_REFUTED_LEAVES = 1000
MIN_CUBES = 0
MAX_CUBES = 50000000
MAX_MARCH_TIME = 86400.0
RANDOM_SAMPLE_SIZE = 100
SOLVER_TIME_LIMIT = 5000
cnf_name = ''
stat_name = ''
start_time = 0.0

class random_cube_data:
	cube_cnf_name = ''
	solved_tasks = 0

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
	lines = []
	random_cubes = []
	remaining_cubes_str = []
	with open(cubes_name, 'r') as cubes_file:
		lines = cubes_file.readlines()
		if len(lines) > RANDOM_SAMPLE_SIZE:
			random_lines = random.sample(lines, RANDOM_SAMPLE_SIZE)
			for line in random_lines:
				lst = line.split(' ')[1:-1] # skip 'a' and '0'
				random_cubes.append(lst)
			remaining_cubes_str = [line for line in lines if line not in random_lines]
		else:
			logging.error('skip n: number of cubes is smaller than random sample size')

	if len(random_cubes) > 0 and len(random_cubes) + len(remaining_cubes_str) != len(lines):
		logging.error('incorrect number of of random and remaining cubes')
		exit(1)
	return random_cubes, remaining_cubes_str
	
def process_n(n : int, cnf_name : str):
	print('n : %d' % n)
	start_t = time.time()
	cubes_name = './cubes_n_' + str(n) + '_' + cnf_name.replace('./','').replace('.cnf','')
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
			random_cubes, remaining_cubes_str = get_random_cubes(cubes_name)
			if len(random_cubes) > 0: # if random sample is small enough to obtain it
				random_cubes_n[n] = random_cubes
				# write all cubes which are not from the random sample to solve them further in the case n is the best one
				with open(cubes_name, 'w') as remaining_cubes_file:
					for cube in remaining_cubes_str:
						remaining_cubes_file.write(cube)
	else:
		remove_file(cubes_name)
	if cubes_num > MAX_CUBES or march_time > MAX_MARCH_TIME:
		is_exit = True
		logging.info('is_exit : ' + str(is_exit))
	
def process_cube_solver(cnf_name : str, n : int, cube : list, cube_index : int, task_index : int, solver : str):
	known_cube_cnf_name = './sample_cnf_n_' + str(n) + '_cube_' + str(cube_index) + '_task_' + str(task_index) + '.cnf'
	p_c.add_cube(cnf_name, known_cube_cnf_name, cube)
	if '.sh' in solver:
		sys_str = solver + ' ' + known_cube_cnf_name + ' ' + str(task_index) + ' ' + str(SOLVER_TIME_LIMIT)
	else:
		sys_str = './timelimit -T 1 -t ' + str(SOLVER_TIME_LIMIT) + ' ' + solver + ' ' + known_cube_cnf_name
	#print('system command : ' + sys_str)
	t = time.time()
	o = os.popen(sys_str).read()
	t = time.time() - t
	solver_time = float(t)
	isSat = p_c.find_sat_log(o)
	if isSat:
		sat_name = cnf_name.replace('./','').replace('.cnf','') + '_' + solver + '_cube_index_' + str(cube_index) 
		sat_name = sat_name.replace('./','')
		with open('!sat_' + sat_name, 'w') as ofile:
			ofile.write('*** SAT found\n')
			ofile.write(o)
	else:
		# remove cnf with known cube
		remove_file(known_cube_cnf_name)
		# remove tmp files from solver's script
		remove_file('./id-' + str(task_index) + '-*')
		
	return n, cube_index, solver, solver_time, isSat
	
def collect_cube_solver_result(res):
	global results
	n = res[0]
	cube_index = res[1]
	solver = res[2]
	solver_time = res[3]
	isSat = res[4]
	results[n].append((cube_index,solver,solver_time)) # append a tuple
	logging.info('n : %d, got %d results - cube_index %d, solver %s, time %f' % (n, len(results[n]), cube_index, solver, solver_time))

	if isSat:
		logging('*** SAT found')
		logging(res)
		elapsed_time = time.time() - start_time
		logging.info('elapsed_time : ' + str(elapsed_time))
		exit(1)
	
if __name__ == '__main__':
	cpu_number = mp.cpu_count()

	is_exit = False

	if len(sys.argv) < 2:
		print('Usage : prog cnf-name [--nosample | --onesample]')
		exit(1)
	cnf_name = sys.argv[1]

	log_name = './find_n_' + cnf_name.replace('./','').replace('.','') + '.log'
	print('log_name : ' + log_name)
	logging.basicConfig(filename=log_name, filemode = 'w', level=logging.INFO)

	logging.info('cnf : ' + cnf_name)
	logging.info("total number of processors: %d" % mp.cpu_count())
	logging.info('cpu_number : %d' % cpu_number)

	is_unsat_sample_solving = True
	is_one_sample = False
	if len(sys.argv) > 2:
		if sys.argv[2] == '--nosample':
			is_unsat_sample_solving = False
		elif sys.argv[2] == '--onesample':
			is_one_sample = True
	logging.info('is_unsat_sample_solving : ' + str(is_unsat_sample_solving))
	logging.info('is_one_sample : ' + str(is_one_sample))
	
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
	# use 1 CPU core if many cubes (much RAM)
	if MAX_CUBES > 5000000:
		pool = mp.Pool(1)
	else:
		pool = mp.Pool(cpu_number)
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

	pool.close()
	pool.join()

	pool2 = mp.Pool(cpu_number)
	
	if is_unsat_sample_solving:
		# prepare file for results
		sample_name = 'sample_results_' + cnf_name
		sample_name = sample_name.replace('.','')
		sample_name = sample_name.replace('/','')
		sample_name += '.csv'
		with open(sample_name, 'w') as sample_file:
			sample_file.write('n cube-index solver time\n')
		# sort dict by n in descending order
		sorted_random_cubes_n = collections.OrderedDict(sorted(random_cubes_n.items()))
		# if only the sample for the first (easiest) n is needed
		if is_one_sample:
			sorted_random_cubes_n = sorted_random_cubes_n.popitem(last=False)
		logging.info('sorted_random_cubes_n : ')
		logging.info(sorted_random_cubes_n)
		# for evary n solve cube-problems from the random sample
		logging.info('')
		logging.info('processing random samples')
		logging.info('')
		
		results = dict()
		for n, random_cubes in sorted_random_cubes_n.items():
			logging.info('*** n : %d' % n)
			logging.info('random_cubes size : %d' % len(random_cubes))
			results[n] = []
			results_size = len(random_cubes) * len(p_c.solvers)
			logging.info('results size : %d' % results_size)
			cube_index = 0
			
			task_index = 0
			for cube in random_cubes:
				for solver in p_c.solvers:
					pool2.apply_async(process_cube_solver, args=(cnf_name, n, cube, cube_index, task_index, solver), callback=collect_cube_solver_result)
					task_index += 1
				cube_index += 1
			# wait for all results
			while len(results[n]) < results_size:
				time.sleep(5)
			logging.info('results[n] len : %d' % len(results[n]))
			logging.info(results[n])
			elapsed_time = time.time() - start_time
			logging.info('elapsed_time : ' + str(elapsed_time) + '\n')
			# write to result file
			with open(sample_name, 'a') as sample_file:
				for res in results[n]:
					sample_file.write('%d %d %s %.2f\n' % (n, res[0], res[1], res[2])) # tuple (cube_index,solver,solver_time)

		pool2.close()
		pool2.join()

	elapsed_time = time.time() - start_time
	logging.info('elapsed_time : ' + str(elapsed_time))

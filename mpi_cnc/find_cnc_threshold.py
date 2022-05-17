import sys
import os
import time
import multiprocessing as mp
import random
import collections
import logging
import time
from enum import Enum

version = "1.2.5"

# Constants:
LA_SOLVER = 'march_cu'
MAX_CUBES_PARALLEL = 5000000
SOLVERS = ['kissat_sc2021']

cnf_name = ''
stat_name = ''
start_time = 0.0

class Mode(Enum):
	eager = 0
	relaxed = 1

class Options:
	sample_size = 1000
	min_cubes = 1000
	max_cubes = 1000000
	min_refuted_leaves = 500
	max_la_time = 86400
	max_cdcl_time = 5000
	nstep = 10
	isrelaxed = False
	seed = 0
	def __init__(self):
		self.seed = round(time.time() * 1000)
	def __str__(self):
		return 'sample_size : ' + str(self.sample_size) + '\n' +\
		'min_cubes : ' + str(self.min_cubes) + '\n' +\
		'max_cubes : ' + str(self.max_cubes) + '\n' +\
		'min_refuted_leaves : ' + str(self.min_refuted_leaves) + '\n' +\
		'max_la_time : ' + str(self.max_la_time) + '\n' +\
		'max_cdcl_time : ' + str(self.max_cdcl_time) + '\n' +\
		'nstep : ' + str(self.nstep) + '\n' +\
		'seed : ' + str(self.seed) + '\n' +\
		'isrelaxed : ' + str(self.isrelaxed) + '\n'
	def read(self, argv) :
		for p in argv:
			if '-sample=' in p:
				self.sample_size = int(p.split('-sample=')[1])
			if '-minc=' in p:
				self.min_cubes = int(p.split('-minc=')[1])
			if '-maxc=' in p:
				self.max_cubes = int(p.split('-maxc=')[1])
			if '-minref=' in p:
				self.min_refuted_leaves = int(p.split('-minref=')[1])
			if '-maxlat=' in p:
				self.max_la_time = int(p.split('-maxlat=')[1])
			if '-maxcdclt=' in p:
				self.max_cdcl_time = int(p.split('-maxcdclt=')[1])
			if '-nstep=' in p:
				self.nstep = int(p.split('-nstep=')[1])
			if '-seed=' in p:
				self.seed = int(p.split('-seed=')[1])
			if p == '--relaxed':
				self.isrelaxed = True

def kill_unuseful_processes():
	sys_str = 'killall -9 ' + LA_SOLVER
	o = os.popen(sys_str).read()
	sys_str = 'killall -9 timelimit'
	o = os.popen(sys_str).read()

def kill_solver(solver):
	sys_str = 'killall -9 ' + solver.replace('./','')
	o = os.popen(sys_str).read()

def remove_file(file_name):
	sys_str = 'rm -f ' + file_name
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

def parse_cubing_log(o):
	cubes = -1
	refuted_leaves = -1
	lines = o.split('\n')
	for line in lines:
		if 'c number of cubes' in line:
			cubes = int(line.split('c number of cubes ')[1].split(',')[0])
			refuted_leaves = int(line.split(' refuted leaves')[0].split(' ')[-1])
	return cubes, refuted_leaves

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

def get_random_cubes(cubes_name):
	global op
	lines = []
	random_cubes = []
	remaining_cubes_str = []
	with open(cubes_name, 'r') as cubes_file:
		lines = cubes_file.readlines()
		if len(lines) > op.sample_size:
			random_lines = random.sample(lines, op.sample_size)
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
	
def process_n(n : int, cnf_name : str, op : Options):
	print('n : %d' % n)
	start_t = time.time()
	cubes_name = './cubes_n_' + str(n) + '_' + cnf_name.replace('./','').replace('.cnf','')
	system_str = 'timelimit -T 1 -t ' + str(int(op.max_la_time)) +  ' ' + LA_SOLVER + ' ' + cnf_name + \
	' -n ' + str(n) + ' -o ' + cubes_name
	#print('system_str : ' + system_str)
	o = os.popen(system_str).read()
	t = time.time() - start_t
	cubes_num = -1
	refuted_leaves = -1
	cubing_time = -1.0
	cubes_num, refuted_leaves = parse_cubing_log(o)
	cubing_time = float(t)
	#print('elapsed_time : %.2f' % elapsed_time)
	return n, cubes_num, refuted_leaves, cubing_time, cubes_name

def collect_n_result(res):
	global random_cubes_n
	global exit_cubes_creating
	global op
	n = res[0]
	cubes_num = res[1]
	refuted_leaves = res[2]
	cubing_time = res[3]
	cubes_name = res[4]
	if cubes_num >= op.min_cubes and cubes_num <= op.max_cubes and cubes_num >= op.sample_size and refuted_leaves >= op.min_refuted_leaves:
		logging.info(res)
		ofile = open(stat_name,'a')
		ofile.write('%d %d %d %.2f\n' % (n, cubes_num, refuted_leaves, cubing_time))
		ofile.close()
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
	if cubes_num > op.max_cubes or cubing_time > op.max_la_time:
		exit_cubes_creating = True
		logging.info('exit_cubes_creating : ' + str(exit_cubes_creating))

def process_cube_solver(cnf_name : str, n : int, cube : list, cube_index : int, task_index : int, solver : str):
	global op
	known_cube_cnf_name = './sample_cnf_n_' + str(n) + '_cube_' + str(cube_index) + '_task_' + str(task_index) + '.cnf'
	add_cube(cnf_name, known_cube_cnf_name, cube)
	if '.sh' in solver:
		sys_str = solver + ' ' + known_cube_cnf_name + ' ' + str(op.max_cdcl_time)
	else:
		sys_str = 'timelimit -T 1 -t ' + str(op.max_cdcl_time) + ' ' + solver + ' ' + known_cube_cnf_name
	t = time.time()
	o = os.popen(sys_str).read()
	t = time.time() - t
	solver_time = float(t)
	isSat = find_sat_log(o)
	if isSat:
		logging.info('*** Writing satisfying assignment to a file')
		sat_name = cnf_name.replace('./','').replace('.cnf','') + '_n' + str(n) + '_' + solver + '_cube_index_' + str(cube_index) 
		sat_name = sat_name.replace('./','')
		with open('!sat_' + sat_name, 'w') as ofile:
			ofile.write('*** SAT found\n')
			ofile.write(o)
	else:
		# remove cnf with known cube
		remove_file(known_cube_cnf_name)
	return n, cube_index, solver, solver_time, isSat

def collect_cube_solver_result(res):
	global results
	global stopped_solvers
	global op
	n = res[0]
	cube_index = res[1]
	solver = res[2]
	solver_time = res[3]
	isSat = res[4]
	results[n].append((cube_index,solver,solver_time)) # append a tuple
	logging.info('n : %d, got %d results - cube_index %d, solver %s, time %f' % (n, len(results[n]), cube_index, solver, solver_time))
	if isSat:
		logging.info('*** SAT found')
		logging.info(res)
		elapsed_time = time.time() - start_time
		logging.info('elapsed_time : ' + str(elapsed_time))
	elif solver_time >= op.max_cdcl_time and not op.isrelaxed:
		logging.info('*** CDCL solver reached time limit, so interrupt')
		logging.info(res)
		elapsed_time = time.time() - start_time
		logging.info('elapsed_time : ' + str(elapsed_time))
		stopped_solvers.add(solver)
		logging.info('stopped solvers : ')
		logging.info(stopped_solvers)

if __name__ == '__main__':
	cpu_number = mp.cpu_count()
	exit_cubes_creating = False

	if len(sys.argv) < 2:
		print('Usage : script cnf-name [options]')
		print('options :\n' +\
		'-sample=x   - (default : 1000)    random sample size' + '\n' +\
		'-minc=x     - (default : 1000)    minimal number of cubes' + '\n' +\
		'-maxc=x     - (default : 1000000) maximal number of cubes' + '\n' +\
		'-minref=x   - (default : 500)     minimal number of refuted leaves' + '\n' +\
		'-maxlat=x   - (default : 86400)   time limit in seconds for lookahead solver' + '\n' +\
		'-maxcdclt=x - (default : 5000)    time limit in seconds for CDCL solver' + '\n' +\
		'-nstep=x    - (default : 10)      step for decreasing threshold n for lookahead solver' + '\n' +\
		'-seed=x     - (default : time)    seed for pseudorandom generator' + '\n' +\
		'--relaxed   - (default : False)   do not stop if CDCL solver is interrupted')
		exit(1)
	cnf_name = sys.argv[1]

	op = Options()
	op.read(sys.argv[2:])
	print(op)

	random.seed(op.seed)

	log_name = './log_' + cnf_name.replace('./','').replace('.','')
	print('log_name : ' + log_name)
	logging.basicConfig(filename=log_name, filemode = 'w', level=logging.INFO)

	logging.info('cnf : ' + cnf_name)
	logging.info('total number of processors: %d' % mp.cpu_count())
	logging.info('cpu_number : %d' % cpu_number)
	logging.info('Options: \n' + str(op))

	start_time = time.time()

	# Count free variables:
	free_vars = get_free_vars(cnf_name)
	logging.info('free vars : %d' % len(free_vars))
	n = len(free_vars)
	while n % op.nstep != 0 and n > 0:
		n -= 1
	logging.info('start n : %d ' % n)

	# prepare an output file
	stat_name = 'stat_' + cnf_name
	stat_name = stat_name.replace('.','')
	stat_name = stat_name.replace('/','')
	stat_file = open(stat_name,'w')
	stat_file.write('n cubes refuted-leaves cubing-time\n')
	stat_file.close()

	random_cubes_n = dict()
	# use 1 CPU core if many cubes (much RAM)
	if op.max_cubes > MAX_CUBES_PARALLEL:
		pool = mp.Pool(1)
	else:
		pool = mp.Pool(cpu_number)
	# find required n and their cubes numbers
	while not exit_cubes_creating:
		pool.apply_async(process_n, args=(n, cnf_name, op), callback=collect_n_result)
		while len(pool._cache) >= cpu_number: # wait until any cpu is free
			time.sleep(2)
		n -= op.nstep
		if exit_cubes_creating or n <= 0:
			#pool.terminate()
			logging.info('killing unuseful processes')
			kill_unuseful_processes()
			time.sleep(2) # wait for processes' termination
			break
	
	elapsed_time = time.time() - start_time
	logging.info('elapsed_time : ' + str(elapsed_time))
	logging.info('random_cubes_n : ')

	pool.close()
	pool.join()

	pool2 = mp.Pool(cpu_number)
	
	# prepare file for results
	sample_name = 'sample_results_' + cnf_name
	sample_name = sample_name.replace('.','')
	sample_name = sample_name.replace('/','')
	sample_name += '.csv'
	with open(sample_name, 'w') as sample_file:
		sample_file.write('n cube-index solver time\n')
	# sort dict by n in descending order
	sorted_random_cubes_n = collections.OrderedDict(sorted(random_cubes_n.items()))
	
	logging.info('sorted_random_cubes_n : ')
	logging.info(sorted_random_cubes_n)
	# for evary n solve cube-problems from the random sample
	logging.info('')
	logging.info('processing random samples')
	logging.info('')
	
	stopped_solvers = set()
	results = dict()
	for n, random_cubes in sorted_random_cubes_n.items():
		logging.info('*** n : %d' % n)
		logging.info('random_cubes size : %d' % len(random_cubes))
		results[n] = []
		task_index = 0
		for solver in SOLVERS:
			if solver in stopped_solvers:
				continue
			cube_index = 0
			exit_solving = False
			for cube in random_cubes:
				while len(pool2._cache) >= cpu_number:
					time.sleep(2)
				# Break if solver becomes a stopped one:
				if solver in stopped_solvers:
					# Kill only a binary solver, let a script solver finisn and clean:
					if '.sh' not in solver:
						kill_solver(solver)
					break
				pool2.apply_async(process_cube_solver, args=(cnf_name, n, cube, cube_index, task_index, solver), callback=collect_cube_solver_result)
				task_index += 1
				cube_index += 1
		time.sleep(2)
		logging.info('results[n] len : %d' % len(results[n]))
		#logging.info(results[n])
		elapsed_time = time.time() - start_time
		logging.info('elapsed_time : ' + str(elapsed_time) + '\n')
		
		if len(stopped_solvers) == len(SOLVERS):
			logging.info('stop main loop')
			break

	pool2.close()
	pool2.join()

	# write results
	for n, res in results.items():
		with open(sample_name, 'a') as sample_file:
			for r in res:
				sample_file.write('%d %d %s %.2f\n' % (n, r[0], r[1], r[2])) # tuple (cube_index,solver,solver_time)

	# remove tmp files from solver's script
	remove_file('./*.mincnf')
	remove_file('./*.cubes')
	remove_file('./*.ext')
	remove_file('./*.icnf')

	elapsed_time = time.time() - start_time
	logging.info('elapsed_time : ' + str(elapsed_time))

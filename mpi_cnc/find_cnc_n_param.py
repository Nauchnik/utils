import sys
import os
import time
import multiprocessing as mp
import random

MIN_REFUTED_LEAVES = 1000
MAX_CUBES = 10000000
MIN_MARCH_TIME = 60.0
MAX_MARCH_TIME = 3600.0
UNSAT_RANDOM_SAMPLE_SIZE = 100
cnfname = ''
statname = ''
results = []

def remove_file(file_name):
	sys_str = 'rm ' + file_name
	o = os.popen(sys_str).read()

def get_free_vars(cnfname):
	free_vars = []
	with open(cnfname) as cnf:
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
	remove_file(cubes_name)
	random_cubes = random.sample(cubes, UNSAT_RANDOM_SAMPLE_SIZE)
	return random_cubes
	
def process_n(n : int, cnfname : str):
	print('n : %d' % n)
	start_time = time.time()
	cubes_name = './cubes_n_' + str(n) + '_' + cnfname.replace('./','')
	system_str = './timelimit -T 1 -t ' + str(MAX_MARCH_TIME) +  ' ./march_cu ' + cnfname + \
	' -n ' + str(n) + ' -o ' + cubes_name
	#print('system_str : ' + system_str)
	o = os.popen(system_str).read()
	elapsed_time = time.time() - start_time
	cubes_num = -1
	refuted_leaves = -1
	cubes_num, refuted_leaves = parse_march_log(o)
	march_time = float(elapsed_time)
	print('elapsed_time : %.2f' % elapsed_time)
	
	random_cubes = []
	if march_time < MAX_MARCH_TIME:
		random_cubes = get_random_cubes(cubes_name)
	
	return n, cubes_num, refuted_leaves, march_time, random_cubes

def collect_result(res):
	print(res[:-1])
	results.append(res)
	n = res[0]
	cubes_num = res[1]
	refuted_leaves = res[2]
	march_time = res[3]
	random_cubes = res[4]
	global is_exit
	if cubes_num > MAX_CUBES or march_time > MAX_MARCH_TIME:
		is_exit = True
		print('is_exit : ' + str(is_exit))
	elif refuted_leaves >= MIN_REFUTED_LEAVES and march_time >= MIN_MARCH_TIME:
		ofile = open(statname,'a')
		ofile.write('%d %d %d %.2f\n' % (n, cubes_num, refuted_leaves, march_time))
		ofile.close()
		#print(random_cubes)
	
if __name__ == '__main__':
	print("total number of processors: ", mp.cpu_count())
	cpu_number = mp.cpu_count()
	if cpu_number > 16:
		cpu_number = int(cpu_number/2)
	print('cpu_number : %d' % cpu_number)
	pool = mp.Pool(cpu_number)
	is_exit = False
	pool = mp.Pool(cpu_number)

	if len(sys.argv) < 2:
		print('Usage : prog cnf-name')
		exit(1)
	cnfname = sys.argv[1]
	print('cnf : ' + cnfname)
	start_time = time.time()

	# count free variables
	free_vars = get_free_vars(cnfname)
	print('free vars : %d' % len(free_vars))
	n = len(free_vars)
	while n % 10 != 0:
		n -= 1
	print('start n : %d ' % n)

	# prepare an output file
	statname = 'stat_' + cnfname
	statname = statname.replace('.','')
	statname = statname.replace('/','')
	ofile = open(statname,'w')
	ofile.write('n cubes refuted-leaves march-cu-time\n')
	ofile.close()

	while not is_exit:
		pool.apply_async(process_n, args=(n, cnfname), callback=collect_result)
		while len(pool._cache) >= cpu_number: # wait until any cpu is free
			time.sleep(2)
		n -= 10
		if is_exit or n <= 0:
			print('terminating pool')
			pool.terminate()
			break
	
	pool.close()
	pool.join()

	elapsed_time = time.time() - start_time
	print('elapsed_time : ' + str(elapsed_time))
	print(results)

	# kill processes
	system_str = 'killall -9 march_cu'
	print('system_str : ' + system_str)
	o = os.popen(system_str).read()
	time.sleep(2)
	o = os.popen(system_str).read()

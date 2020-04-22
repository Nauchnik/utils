import sys
import os
import time
import multiprocessing as mp

MIN_REFUTED_CUBES = 1000
MAX_NON_REFUTED_CUBES = 10000000
MAX_MARCH_TIME = 86400
cnfname = ''
statname = ''
results = []

def process_n(n : int, cnfname : str):
	print('n : %d' % n)
	start_time = time.time()
	system_str = './timelimit -T 1 -t ' + str(MAX_MARCH_TIME) +  ' ./march_cu ' + cnfname + ' -n ' + str(n) 
	#print('system_str : ' + system_str)
	o = os.popen(system_str).read()
	elapsed_time = time.time() - start_time
	s = ''
	for x in o:
		s += x
	lst = s.split('\n')
	cubes = 0
	refuted = 0
	perc_refuted = 0.0
	for x in lst:
		if 'c number of cubes' in x:
			non_refuted = int(x.split('c number of cubes ')[1].split(',')[0])
			refuted = int(x.split(' refuted leaves')[0].split(' ')[-1])
			cubes = non_refuted + refuted
			print('non_refuted cubes : %d ' % non_refuted)
			print('refuted cubes : %d' % refuted)
			if cubes > 0:
				perc_refuted = refuted*100/cubes

	return n, cubes, refuted, non_refuted, perc_refuted, int(elapsed_time)

def collect_result(res):
	results.append(res)
	if res[2] >= MIN_REFUTED_CUBES:
		ofile = open(statname,'a')
		ofile.write('%d %d %d %d %f %d\n' % (res[0], res[1], res[2], res[3], res[4], res[5]))
		ofile.close()
	global is_exit
	if res[3] > MAX_NON_REFUTED_CUBES or res[5] > MAX_MARCH_TIME:
		is_exit = True
	
if __name__ == '__main__':
	print("total number of processors: ", mp.cpu_count())
	cpu_number = mp.cpu_count()
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
	free_vars = []
	with open(cnfname) as cnf:
		lines = cnf.readlines()
		for line in lines:
			if line[0] == 'p' or line[0] == 'c':
				continue
			lst = line.split(' ')
			#print(line)
			for x in lst:
				if x == ' ' or x == '':
					continue
				var = abs(int(x))
				if var != 0 and var not in free_vars:
					free_vars.append(var)
	print('free vars : %d' % len(free_vars))

	# prepair an output file
	statname = 'stat_' + cnfname
	statname = statname.replace('.','')
	statname = statname.replace('/','')
	ofile = open(statname,'w')
	ofile.write('n cubes non-refuted-cubes refuted-cubes %-refuted-cubes time\n')
	ofile.close()

	n = len(free_vars)
	while n % 10 != 0:
		n -= 1
	print('start n : %d ' % n)

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

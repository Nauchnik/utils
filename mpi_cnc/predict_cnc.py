import sys
import os
import time
from random import randint

def generate_random_values(template_cnf_name : str, cnf_id : int):
	values = dict()
	sys_str = './lingeling --seed=' + str(cnf_id) + ' ' + template_cnf_name
	print('start system command : ' + sys_str)
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
				print('cnf_vars_number : %d' % cnf_vars_number)
			else:
			    template_cnf_clauses.append(line)
	#
	cnf_clauses_number = len(template_cnf_clauses) + len(known_vars_values)
	print('cnf_clauses_number : %d' % cnf_clauses_number)
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
	print('start system command : ' + sys_str)
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
			print(line)
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
		print('clauses_number : %d' % clauses_number)
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

def make_cnf_known_sat_cube(n: int, cnf_name : str, values_all_vars : list):
	min_cnf_name = 'min_' + cnf_name
	sys_str = './lingeling -s -T 60 -o ' + min_cnf_name + ' ' + cnf_name
	print('start system command : ' + sys_str)
	o = os.popen(sys_str).read()
	isSat = find_sat_log(o)
	if isSat:
		print('*** SAT found by lingeling')
		print(o)
		exit(1)
	remove_file(cnf_name)
	cubes_name = 'cubes_' + min_cnf_name
	sys_str = './march_cu ' + min_cnf_name + ' -n ' + str(n) + ' -o ' + cubes_name
	print('start system command : ' + sys_str)
	o = os.popen(sys_str).read()
	isSat = find_sat_log(o)
	if isSat:
		print('*** SAT found by march_cu')
		print(o)
		exit(1)
	march_data = find_refuted(o)
	print(o)
	sat_cubes = get_sat_cube(cubes_name, values_all_vars)
	print('%d sat_cubes :' % len(sat_cubes))
	if len(sat_cubes) == 0:
		exit(1)
	print(sat_cubes)
	cnf_known_sat_cube_name = 'known_sat_cube_' + min_cnf_name
	add_cube(min_cnf_name, cnf_known_sat_cube_name, sat_cubes[0])
	remove_file(min_cnf_name)
	return cnf_known_sat_cube_name, march_data[0], march_data[1]

def get_solving_time(o):
	#cadical - c total real time since initialization: 
	#maple_v3 - c CPU time              :
	solving_time = 0.0 
	lines = o.split('\n')
	for line in lines:
		if 'c total real time' in line:
			print(line)
			solving_time = float(line.split()[6])
		elif 'c CPU time' in line:
			print(line)
			solving_time = float(line.split()[4])
	return solving_time

def solve_cnf_id(solvers : list, template_cnf_name : str, cnf_id : int):
	print('cnf_id : %d' % cnf_id)
	values_all_vars = generate_random_values(template_cnf_name, cnf_id)
	print('%d values_all_vars : ' % len(values_all_vars))
	#print(values_all_vars)
	output_vars = dict()
	for i in range(len(values_all_vars)-output_vars_number,len(values_all_vars)):
		v = i+1
		output_vars[v] = values_all_vars[v]
	print('%d output vars :' % len(output_vars))
	cnf_name = 'rand_' + template_cnf_name.replace('./','').split('.')[0] + '_cnfid_' + str(cnf_id) + '.cnf'
	make_cnf_known_values(template_cnf_name, cnf_name, output_vars)
	data = make_cnf_known_sat_cube(n, cnf_name, values_all_vars)
	cnf_known_sat_cube_name = data[0]
	cubes = data[1]
	refuted = data[2]
	solvers_times = dict()
	for solver in solvers:
		sys_str = solver + ' ' + cnf_known_sat_cube_name
		print('start system command : ' + sys_str)
		o = os.popen(sys_str).read()
		print(o)
		solvers_times[solver] = get_solving_time(o)
	remove_file(cnf_known_sat_cube_name)
	return cubes, refuted, solvers_times

def collect_result(result):
    global results
    results.append(result)

#template_cnf_name = sys.argv[1]
template_cnf_name = 'md4_40_with_constr_template.cnf'
print('template_cnf_name : ' + template_cnf_name)

input_vars_number = 512
output_vars_number = 128 # last variables in a CNF
n = 2710
random_sample_size = 100
solvers = ['./MapleLCMDistChrBt-DL-v3', './cadical_sr2019']
cpu_number = 12

start_time = time.time()

print('solvers :')
print(solvers)
print('random sample size : %d' % random_sample_size)
print('cpu_number : %d' % cpu_number)

import multiprocessing as mp
#print("Number of processors: ", mp.cpu_count())

pool = mp.Pool(cpu_number)
results = []
for cnf_id in range(random_sample_size):
	pool.apply_async(solve_cnf_id, args=(solvers, template_cnf_name, cnf_id), callback=collect_result)
pool.close()
pool.join()
print('results: ')
for r in results:
	print(r)

elapsed_time = time.time() - start_time
print('elapsed_time : ' + str(elapsed_time))
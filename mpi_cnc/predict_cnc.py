import sys
import os
import time
from random import randint

input_vars_number = 512
output_vars_number = 128 # last variables in a CNF

def generate_random_values(template_cnf_name : str, cnf_id : int):
	values = dict()
	sys_str = './lingeling/lingeling --seed=' + str(cnf_id) + ' ' + template_cnf_name
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

def make_cnf_known_values(template_cnf_name : str, cur_cnf_name : str, known_vars_values : dict):
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
	with open(cur_cnf_name, 'w') as cnf:
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

template_cnf_name = sys.argv[1]
#template_cnf_name = "md4_40_zero_hash_with_constr.cnf"
print('template_cnf_name : ' + template_cnf_name)

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

def add_cube(old_cnf_name : str, new_cnf_name : str, it : int, cube : list):
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

start_time = time.time()


# generete a random 512-bit input that feets given constraints
#values_input_vars = [randint(0, 1) for p in range(len(input_vars))]
cnf_id = 0
values_all_vars = generate_random_values(template_cnf_name, cnf_id)
print('%d values_all_vars : ' % len(values_all_vars))
#print(values_all_vars)

input_vars = dict()
for i in range(input_vars_number):
	v = i+1
	input_vars[v] = values_all_vars[v]
print('%d input vars values :' % len(input_vars))
#print(input_vars)

output_vars = dict()
for i in range(len(values_all_vars)-output_vars_number,len(values_all_vars)):
	v = i+1
	output_vars[v] = values_all_vars[v]

print('%d output vars :' % len(output_vars))
print(output_vars)

isSat = False
it = 0
sat_cubes = []
min_cur_cnf_name = ''
while not isSat:
	print('')
	print('*** iteration : %d' % it)
	cur_cnf_name = 'rand_' + template_cnf_name.replace('./','').split('.')[0] + '_cnfid_' + str(cnf_id) + '_it_' + str(it) + '.cnf'
	if it == 0:
			# make CNF with known output
			make_cnf_known_values(template_cnf_name, cur_cnf_name, output_vars)
	elif it > 0:
			print('adding cube to a CNF')
			add_cube(min_cur_cnf_name, cur_cnf_name, it, sat_cubes[0])

	min_cur_cnf_name = 'min_' + cur_cnf_name
	sys_str = './lingeling/lingeling -s -T 60 -o ' + min_cur_cnf_name + ' ' + cur_cnf_name
	print('start system command : ' + sys_str)
	o = os.popen(sys_str).read()
	# remove current cnf after its minimization
	remove_file(cur_cnf_name)
	isSat = find_sat_log(o)
	if isSat:
		print('*** SAT found by lingeling')
		print(o)
		break
	#print(o)
	sys_str = 'python3 ./find_cnc_n_param.py ./' + min_cur_cnf_name
	print('start system command : ' + sys_str)
	o = os.popen(sys_str).read()
	isPrint = False
	# print only csv-like date
	lst = o.split('\n')
	for line in lst:
		if 'n cubes non-refuted-cubes refuted-cubes' in line:
			isPrint = True
		if isPrint and len(line) > 1:
			print(line)
	n_param_march = find_n_param(o)
	if n_param_march == -1:
		print('error: n_param_march is -1')
		break
	print('n_param_march : %d' % n_param_march)
	cubes_name = 'cubes_n_' + str(n_param_march) + '_' + min_cur_cnf_name
	sys_str = './march_cu/march_cu ' + min_cur_cnf_name + ' -n ' + str(n_param_march) + ' -o ' + cubes_name 
	print('start system command : ' + sys_str)
	o = os.popen(sys_str).read()
	isSat = find_sat_log(o)
	if isSat:
		print('*** SAT found by march_cu')
		print(o)
		break
	#print(o)
	sat_cubes = get_sat_cube(cubes_name, values_all_vars)
	print('%d sat_cubes :' % len(sat_cubes))
	if len(sat_cubes) == 0:
		exit(1)
	print(sat_cubes)
	it += 1

elapsed_time = time.time() - start_time
print('elapsed_time : ' + str(elapsed_time))
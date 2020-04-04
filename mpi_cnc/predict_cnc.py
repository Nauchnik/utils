import sys
import os
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

def make_cnf_known_values(template_cnf_name : str, new_cnf_name : str, known_vars_values : dict):
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
	with open(new_cnf_name, 'w') as cnf:
		cnf.write('p cnf ' + str(cnf_vars_number) + ' ' + str(cnf_clauses_number) + '\n')
		for clause in template_cnf_clauses:
			cnf.write(clause)
		for var in known_vars_values:
			s = ''
			if known_vars_values[var] == 0:
				s = '-'
			s += str(var) + ' 0\n'
			cnf.write(s)

template_cnf_name = sys.argv[1]
#template_cnf_name = "md4_40_zero_hash_with_constr.cnf"
print('template_cnf_name : ' + template_cnf_name)

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
print(input_vars)

output_vars = dict()
for i in range(len(values_all_vars)-output_vars_number,len(values_all_vars)):
	v = i+1
	output_vars[v] = values_all_vars[v]

print('%d output vars :' % len(output_vars))
print(output_vars)

# make CNF with known output
new_cnf_name = 'rand_' + str(cnf_id) + '_' + template_cnf_name.replace('./','')
make_cnf_known_values(template_cnf_name, new_cnf_name, output_vars)

sys_str = './find_cnc_n_param.py ./' + new_cnf_name
print('start system command : ' + sys_str)
o = os.popen(sys_str).read()
print(o)
import sys

version = "0.0.2"

cnf_name = ''
MAX_INPUT_VAR = 512

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

if len(sys.argv) < 2:
	print('Usage : prog cnf-name')
	exit(1)
cnf_name = sys.argv[1]

# count free variables
free_vars = get_free_vars(cnf_name)
free_vars = sorted(free_vars)
free_input_vars = []
for var in free_vars:
	if (var <= MAX_INPUT_VAR):
		free_input_vars.append(var)
print(str(len(free_input_vars)) + ' free input vars:')
print(free_input_vars)
print('')
for var in free_input_vars:
    print('v' + str(var) + ' {0,1}[1]')

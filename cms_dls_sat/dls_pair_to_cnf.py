import sys

LS_ORDER = 10

def cnf_var_num(ls_order : int, ls_index : int, row_index : int, col_index : int, cell_val : int):
	return ls_index*pow(ls_order,3) + row_index*pow(ls_order,2) + col_index*ls_order + cell_val + 1

if len(sys.argv) < 3:
	print('Usage : cnf cms [dls_pair]')
	exit(1)

cnfname = sys.argv[1]
cmsname = sys.argv[2]
pairname = ''

print(cnfname)
print(cmsname)

if len(sys.argv) > 3:
	pairname = sys.argv[3]
	print(pairname)

var_num = 0
clauses = []
with open(cnfname, 'r') as cnf:
	lines = cnf.readlines()
	for line in lines:
		if len(line) < 2 or line[0] == 'c':
			continue
		if line[0] == 'p':
			var_num = int(line.split(' ')[2])
		else:
			clauses.append(line)
	

print('')

cms = []
with open(cmsname, 'r') as cmsf:
	lines = cmsf.read().splitlines()
	for line in lines:
	    lst = line.split(' ')
	    row = [int(i) for i in line.split(' ')]
	    cms.append(row)
print(cms)
print('')

clauses_cms = []
for i in range(len(cms)):
	for j in range(len(cms[i])):
		first_dls_cell_vars = [cnf_var_num(LS_ORDER, 0, i, j, k) for k in range(LS_ORDER)]
		i2 = int(cms[i][j] / LS_ORDER)
		j2 = cms[i][j] % LS_ORDER
		second_dls_cell_vars = [cnf_var_num(LS_ORDER, 1, i2, j2, k) for k in range(LS_ORDER)]
		for k in range(len(first_dls_cell_vars)):
			clauses_cms.append(str(first_dls_cell_vars[k]) + ' -' + str(second_dls_cell_vars[k]) + ' 0\n')
			clauses_cms.append('-' + str(first_dls_cell_vars[k]) + ' ' + str(second_dls_cell_vars[k]) + ' 0\n')

clauses_pair = []

if pairname != '':
	first_dls = []
	second_dls = []

	line_num = 0
	with open(pairname, 'r') as f:
		lines = f.read().splitlines()
		for line in lines:
			if len(line) < 2:
				continue
			row = [int(i) for i in line.split(' ')]
			if line_num < LS_ORDER:
				first_dls.append(row)
			else:
				second_dls.append(row)
			line_num += 1

	print(first_dls)
	print('')
	print(second_dls)

	for i in range(len(first_dls)):
		for j in range(len(first_dls[i])):
		    var = cnf_var_num(LS_ORDER, 0, i, j, first_dls[i][j])
		    clauses_pair.append(str(var) + ' 0\n')

	for i in range(len(second_dls)):
		for j in range(len(second_dls[i])):
		    var = cnf_var_num(LS_ORDER, 1, i, j, second_dls[i][j])
		    clauses_pair.append(str(var) + ' 0\n')

if len(clauses_pair) > 0:
	add_name = '_known_pair_cms.cnf'
else:
	add_name = '_known_cms.cnf'

with open(cnfname.replace('.cnf','') + add_name, 'w') as cnf:
	cnf.write('p cnf ' + str(var_num) + ' ' + str(len(clauses) + len(clauses_pair) + len(clauses_cms)) + '\n')
	for clause in clauses_pair:
		cnf.write(clause)
	for clause in clauses_cms:
		cnf.write(clause)
	for clause in clauses:
		cnf.write(clause)
	
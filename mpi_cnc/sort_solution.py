#s SATISFIABLE
#v -2027
#v 6456
#...
#v 875
#v 0

import sys
import binascii

KNOWN_VARS_NUM = 512 

if len(sys.argv) < 3:
	print('Usage: cnf solution')
	exit(1)

cnfname = sys.argv[1]
solname = sys.argv[2]
print('solname : ' + solname)
print('cnfname : ' + cnfname)
literals = []
with open(solname, 'r') as f:
	lines = f.readlines()
	for line in lines:
		if line == '' or line[0] == 's':
			continue
		val = int(line.split(' ')[1])
		if val != 0:
		    literals.append(val)

literals = sorted(literals, key=abs)
#print(literals)
#for i in range(KNOWN_VARS_NUM):
#	print('%d 0' % literals[i])

s = ''

for lit in literals:
	s += str(lit) + ' '

input_bits = ['']
k = 0	
for lit in literals:
	if len(input_bits[k]) == 32:
	    k += 1
	    if k == 16:
		    break
	    input_bits.append('')
	input_bits[k] += '0' if lit < 0 else '1'

print(s)
print('\n')
for x in input_bits:
	print(x)
	print(str(hex(int(x, 2))))

vars_num = 0
clauses_num = 0
clauses = []
with open(cnfname, 'r') as f:
	lines = f.read().splitlines()
	for line in lines:
		if line == '' or line[0] == 'c':
			continue
		if line[0] == 'p':
			vars_num = int(line.split(' ')[2])
			clauses_num = int(line.split(' ')[3])
		else:
			clauses.append(line)

with open(cnfname.split('.cnf')[0] + '_known' + str(KNOWN_VARS_NUM) + '.cnf', 'w') as f:
	f.write('p cnf %d %d\n' % (vars_num, clauses_num + KNOWN_VARS_NUM))
	for i in range(KNOWN_VARS_NUM):
		f.write(str(literals[i]) + ' 0\n')
	for c in clauses:
		f.write(c + '\n')


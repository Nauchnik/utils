#s SATISFIABLE
#v -2027
#v 6456
#...
#v 875
#v 0

import sys

KNOWN_VARS_NUM = 512 

extname = sys.argv[1]
cnfname = sys.argv[2]
print('extname : ' + extname)
print('cnfname : ' + cnfname)
literals = []
with open(extname, 'r') as f:
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
print(s)

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
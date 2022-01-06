# Reads a given output of a SAT solver.
# The output must contain a satysfiyng assignment.
# The assignment is first converted into binary, then into hexadecimal.

import sys

MAX_VAR = 512
BLOCK_SIZE = 32

ifilename = sys.argv[1]
print("ifilename : ", ifilename)

literals = []
with open(ifilename,'r') as f:
	lines = f.read().splitlines()
	
	for line in lines:
		if len(line) > 1 and line[0] == 'v':
			words = line.split(' ')
			for w in words:
				if w != 'v' and w != '':
					literals.append(int(w))
					
print('literals size : ' + str(len(literals)))

k = 0
block = []
literals_blocks = []
for lit in literals[:MAX_VAR]:
	block.append(lit)
	if len(block) == BLOCK_SIZE:
		literals_blocks.append(block)
		block = []

print('literals_blocks size : ', len(literals_blocks))

bin_back_str_lst = []
k = 0
with open(ifilename + '_hex', 'w') as o:
	for literals in literals_blocks:
		s = ''
		for lit in literals:
			s += '1' if lit > 0 else '0'
		o.write(s + '\n')
		bin_back_str_lst.append(s[::-1])
	o.write('\n')
	total_hex_str = ''
	for bbs in bin_back_str_lst:
		h = hex(int(bbs, 2))
		h_str = str(h)
		o.write('X[' + str(k) + '] = ' + h_str + ';\n')
		total_hex_str += h_str + ' '
		k += 1
	o.write('\n' + total_hex_str + '\n')

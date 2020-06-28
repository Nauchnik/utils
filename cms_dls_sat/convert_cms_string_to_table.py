# CMS[0] = 0

import sys

LS_ORDER = 10

fname = sys.argv[1]
print(fname)

cms = dict()
with open(fname, 'r') as f:
	lines = f.read().splitlines()
	for line in lines:
		cms[int(line.split('[')[1].split(']')[0])] = line.split(' = ')[1]
print(cms)

fname_table = fname + '_table'
with open(fname_table , 'w') as f:
	for i in cms:
		f.write(cms[i])
		if (i+1) % LS_ORDER == 0:
			f.write('\n')
		else:
			f.write(' ')
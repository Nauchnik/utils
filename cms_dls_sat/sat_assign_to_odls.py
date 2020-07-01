import sys

LS_ORDER = 10

fname = sys.argv[1]
print(fname)

sat_assign = []
with open(fname, 'r') as f:
	lines = f.read().splitlines()
	for line in lines:
		if len(line) < 2 or line[0] != 'v':
			continue
		lst = line.split(' ')
		for lit in lst[1:]:
			if lit != '0':
				sat_assign.append(int(lit))
#print(sat_assign)

odls_pair = []

row = ''
k = 0
for x in sat_assign:
	if x > 0:
		cell_val = (x-1) % LS_ORDER
		print(cell_val)
		row += str(cell_val) + ' '
		k += 1
		if k == LS_ORDER:
			odls_pair.append(row)
			row = ''
			k = 0
			if len(odls_pair) == LS_ORDER:
				odls_pair.append('')
				
for row in odls_pair:
	print(row)
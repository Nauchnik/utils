fname = '!sat_out_id_2328136'

literals = []
with open(fname,'r') as f:
	temp = f.read().splitlines()
	for line in temp:
		if len(line) > 1 and line[0] == 'v':
			words = line.split(' ')
			print(words)
			for w in words:
				if w != 'v':
					literals.append(w)

with open('out', 'w') as o:
	for l in literals[:512]:
		o.write(l + ' 0\n')
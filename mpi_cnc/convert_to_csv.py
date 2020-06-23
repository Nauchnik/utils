# n solver time
# 2450 ./cryptominisat5.7.1 0.01
# 2450 ./cadical130 0.02

import sys
import pandas as pd

fname = sys.argv[1]
print('fname : ' + fname)

solvers_times = dict()
df = pd.read_csv(fname, delimiter = ' ')
print(df)

for index, row in df.iterrows():
	solver = row['solver']
	t = float(row['time'])
	if solver not in solvers_times:
		solvers_times[solver] = []
	solvers_times[solver].append(t)

print(solvers_times)

inst_ids = []
for s in solvers_times:
    print(len(solvers_times[s]))
    inst_ids = [i for i in range(len(solvers_times[s]))]

print(inst_ids)
with open('conv_' + fname.replace('./', '') + '.csv', 'w') as f:
	f.write('instance')
	for solver in solvers_times:
		f.write(' ' + solver.replace('./',''))
	f.write('\n')
	for i in inst_ids:
		s = str(i)
		for solver in solvers_times:
			s += ' ' + str(solvers_times[solver][i])
		f.write(s + '\n')

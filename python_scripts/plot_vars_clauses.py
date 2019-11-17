import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import zipfile

ordinary_bench_zip_name = '../benchmarks/58branches_k1-29_delta100-500.zip'
optimized_bench_zip_name = '../benchmarks/reduced_58branches_k1-29_delta100-500.zip.zip'

optimized_bench_zip=zipfile.ZipFile(optimized_bench_zip_name)
file_names = optimized_bench_zip.namelist()
#print(file_names)

#example:
#c limited 0
#c k 1
#c delta 100
#p wcnf 349636 699297 349563

optimized_vars = []
optimized_clauses = []

for f_name in file_names:
	#print(f_name)
	with optimized_bench_zip.open(f_name) as file:
		for line in file:
			#print(line)
			words = str(line).split(' ')
			if 'wcnf' in words:
				#print(line)
				optimized_vars.append(int(words[2]))
				optimized_clauses.append(int(words[3]))
				break
		#	print(line)
print('optimized: ')
print(optimized_vars)
print(optimized_clauses)

ordinary_bench_zip=zipfile.ZipFile(ordinary_bench_zip_name)
file_names = ordinary_bench_zip.namelist()
#print(file_names)

ordinary_vars = []
ordinary_clauses = []

for f_name in file_names:
	#print(f_name)
	with ordinary_bench_zip.open(f_name) as file:
		for line in file:
			#print(line)
			words = str(line).split(' ')
			if 'wcnf' in words:
				#print(line)
				ordinary_vars.append(int(words[2]))
				ordinary_clauses.append(int(words[3]))
				break
		#	print(line)
print(' ')
print('ordinary: ')
print(ordinary_vars)
print(ordinary_clauses)

x_values = []
for i in range(len(ordinary_vars)):
	x_values.append(i)

fig, ax = plt.subplots()
ax.axis([1, 150, 1, 2e6])
ax.plot(x_values, ordinary_clauses, '^--', label='Clauses-Ordinary', color = 'green')
ax.plot(x_values, ordinary_vars, 'h--', label='Vars-Ordinary', color = 'red')
ax.plot(x_values, optimized_clauses, '+--', label='Clauses-Optimized', color = 'purple')
ax.plot(x_values, optimized_vars, 'x--', label='Vars-Optimized', color = 'blue')
#ax.set_xticks(x_values)
#plt.xticks(np.arange(min(x_values), max(x_values)+1, 10.0))
ax.set_yscale("log", nonposy='clip')
ax.set_xlabel('Instance index', rotation='horizontal')
ax.set_ylabel('Size, logarithmic scale', rotation='vertical')
legend = ax.legend(loc='best', shadow=True)
#legend = ax.legend(loc='best', shadow=True, fontsize='x-large')
fig.savefig('encsize.pdf', bbox_inches='tight')
#plt.show()

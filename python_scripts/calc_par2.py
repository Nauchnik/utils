import sys
file_name = sys.argv[1]
print("file name : " + str(file_name))
time_limit = 86400.0
solver_par2 = dict()

with open(file_name) as ifile:
	for str in ifile:
		words = str.split(' ')
		if words[0] == "solver":
			continue
		solver = words[0]
		instance = words[1]
		time = float(words[2])
		if solver not in solver_par2:
			solver_par2[solver] = 0.0
		else:
			if time < time_limit:
				solver_par2[solver] += time
			else:
				solver_par2[solver] += time_limit*2
for solver in solver_par2:
	print("solver %s : par2 %f" % (solver, solver_par2[solver]))

# Oleg Zaikin, 2019
# Calculate PAR-2 for SAT solvers' given a log file

import statistics
import sys
import pandas as pd

if len(sys.argv) < 3:
    print("Usage: script log-file time-limit-sec")
    exit(1)

log_file_name = sys.argv[1]
time_limit = int(sys.argv[2])
print("log file : " + log_file_name)
print("time limit in sec : " + str(time_limit))

log_file_df = pd.read_csv(log_file_name, delimiter = ' ', names=['solver', 'cnf', 'time', 'result'])
#print(log_file)

class solver_result:
    sat = 0
    unsat = 0
    indet = 0
    par2 = 0.0

solvers_dict = dict()

for index, row in log_file_df.iterrows():
    s = row['solver']
    if s not in solvers_dict:
        s_r = solver_result()
        solvers_dict[s] = s_r
    if row['result'] == 'SAT':
        solvers_dict[s].sat += 1
    if row['result'] == 'UNSAT':
        solvers_dict[s].unsat += 1
    if row['result'] == 'INDET':
        solvers_dict[s].indet += 1
    t = float(row['time'])
    solvers_dict[s].par2 += t if t < time_limit else time_limit*2

for solver in solvers_dict:
    print(solver)
    print("solved : %d" % (solvers_dict[solver].sat + solvers_dict[solver].unsat))
    print("sat : %d" % solvers_dict[solver].sat)
    print("unsat : %d" % solvers_dict[solver].unsat)
    print("indet : %d" % solvers_dict[solver].indet)
    print("par2 : %.0f" % solvers_dict[solver].par2)
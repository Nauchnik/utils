# Oleg Zaikin, 2019
# Analyze SAT solvers' outputs in a current folder

import os
from os import listdir
from os.path import isfile, join
import statistics

TIME_LIMIT = 5000

cur_folder = os.getcwd()

outs = [f for f in listdir(cur_folder) if isfile(join(cur_folder, f)) and ".out" in f]
print("%d outputs" % len(outs))

solving_times = []
sat = 0
unsat = 0
#duplicates_cnf = 0
#duplicates_min = 0
#duplicates_cnf_lst = []
#duplicates_min_lst = []
duplicates_lst = []

for f in outs:
    with open(f, 'r') as ifile:
        added_duplicates = 0
        for line in ifile:
            if "s SATISFIABLE" in line:
                sat = sat + 1
            elif "s UNSATISFIABLE" in line:
                unsat = unsat + 1
            elif "c duplicate learnts_cnf : " in line:
                tmp = int(line.split("c duplicate learnts_cnf : ")[1])
                if tmp > 0:
                    added_duplicates = added_duplicates + tmp
                #duplicates_cnf = duplicates_cnf + 1
            elif "c duplicate learnts_min : " in line:
                tmp = int(line.split("c duplicate learnts_min : ")[1])
                if tmp > 0:
                    added_duplicates = added_duplicates + tmp
                #duplicates_min = duplicates_min + 1
            if "c CPU time" in line:
                #print(line)
                lst = line.split("c CPU time              : ")
                if len(lst) > 1:
                    tmp = lst[1].split(" s")[0]
                    solving_times.append(float(tmp))
        #if added_duplicates == 214754103960:
        #    print(f)
        if added_duplicates > 0:
            duplicates_lst.append(added_duplicates)
        
unsolved_count = len(outs) - len(solving_times)

if len(solving_times) + unsolved_count != len(outs):
    print("Error. solved+unsolved!=total")
    exit()

print("solved : %d" % len(solving_times))
print("unsolved : %d" % unsolved_count)
print("sat : %d" % sat)
print("unsat : %d" % unsat)
par2 = sum(solving_times)
par2 = par2 + float(unsolved_count*TIME_LIMIT*2)
par2 = round(par2, 2)
print("par2 : " + str(int(par2)))
print("solved with duplicates : %d" % len(duplicates_lst))
print("duplicates min : %d" % min(duplicates_lst))
print("duplicates max : %d" % max(duplicates_lst))
print("duplicates mean : %d" % int(sum(duplicates_lst)/len(duplicates_lst)))
print("duplicates median : %d" % int(statistics.median(duplicates_lst)))
#print("not noll duplicates : %d" % len(duplicates_lst))
#print("not noll duplicates_cnf : %d" % len(duplicates_cnf_lst))
#print("not noll duplicates_min : %d" % len(duplicates_min_lst))
#print("total duplicates added to core : %d" % (duplicates_cnf + duplicates_min))
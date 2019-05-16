# Oleg Zaikin, 2019
# Analyze SAT solvers' outputs in a current folder

import os
from os import listdir
from os.path import isfile, join
import statistics

TIME_LIMIT = 5000
MAX_DUPLICATE_VALUE = 10000000

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
conflicts_lst = []
instances = []

for f in outs:
    if f not in instances:
        instances.append(f)
    else:
        print("Error. Duplicate instance " + f)
        exit()
    with open(f, 'r') as ifile:
        added_duplicates = 0
        isHugeDuplVal = False
        isSAT = False
        isUNSAT = False
        for line in ifile:
            if "c conflicts             : " in line:
                conflicts = int(line.split("c conflicts             : ")[1].split()[0])
            elif "s SATISFIABLE" in line:
                isSAT = True
            elif "s UNSATISFIABLE" in line:
                isUNSAT = True
            elif "c duplicate learnts_cnf : " in line:
                tmp = int(line.split("c duplicate learnts_cnf : ")[1])
                if tmp > 0:
                    added_duplicates = added_duplicates + tmp
                if tmp > MAX_DUPLICATE_VALUE:
                    isHugeDuplVal = True
                #duplicates_cnf = duplicates_cnf + 1
            elif "c duplicate learnts_min : " in line:
                tmp = int(line.split("c duplicate learnts_min : ")[1])
                if tmp > 0:
                    added_duplicates = added_duplicates + tmp
                if tmp > MAX_DUPLICATE_VALUE:
                    isHugeDuplVal = True
                #duplicates_min = duplicates_min + 1
            if "c CPU time" in line:
                #print(line)
                lst = line.split("c CPU time              : ")
                if len(lst) > 1:
                    tmp = lst[1].split(" s")[0]
                    solving_times.append(float(tmp))
        if isSAT:
            sat = sat + 1
        elif isUNSAT:
            unsat = unsat + 1
        if not isHugeDuplVal and added_duplicates > 0:
            duplicates_lst.append(added_duplicates)
            if conflicts == 0:
                print("Error. conflicts == 0, file " + f)
                exit()
            conflicts_lst.append(conflicts)
        
unsolved_count = len(outs) - len(solving_times)

if len(conflicts_lst) != len(duplicates_lst):
    print("len(conflicts_lst) != len(duplicates_lst)")
    exit()

#print("conflicts_lst len %d" % len(conflicts_lst))

if len(solving_times) + unsolved_count != len(outs):
    print("Error. solved+unsolved!=total")
    exit()

if len(conflicts_lst) > 0:
    mean_added_dupl_perc = 0.0
    for i in range(len(conflicts_lst)):
        mean_added_dupl_perc = mean_added_dupl_perc + duplicates_lst[i] * 100.0 / conflicts_lst[i]
    mean_added_dupl_perc = mean_added_dupl_perc / len(conflicts_lst)
    mean_added_dupl_perc = round(mean_added_dupl_perc, 2)

print("solved : %d" % len(solving_times))
print("unsolved : %d" % unsolved_count)
print("sat : %d" % sat)
print("unsat : %d" % unsat)
par2 = sum(solving_times)
par2 = par2 + float(unsolved_count*TIME_LIMIT*2)
par2 = round(par2, 2)
print("par2 : " + str(int(par2)) + " seconds")
if len(conflicts_lst) > 0:
    print("solved with duplicates : %d" % len(duplicates_lst))
    print("added duplicates min : %d" % min(duplicates_lst))
    print("added duplicates max : %d" % max(duplicates_lst))
    print("added duplicates mean : %d" % int(sum(duplicates_lst)/len(duplicates_lst)))
    print("added duplicates median : %d" % int(statistics.median(duplicates_lst)))
    print("added conflicts mean : %d" % int(sum(conflicts_lst)/len(conflicts_lst)))
    print("added duplicates mean percentage : " + str(mean_added_dupl_perc) + ' %')
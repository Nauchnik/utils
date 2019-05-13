# Oleg Zaikin, 2019
# Generate binaries by varying a given set of parameters

import os
import itertools

basic_solver_folder = "MapleLCMDistChronoBT_v3.7.2"
basic_solver_name = "MapleLCMDistChrBt-v.3.7.2"
pcs_file_name = "dup-solver.pcs"
params_names = []
params_values = []

cur_folder = os.getcwd()
bin_dir = cur_folder + "/binaries/"
print("bin_dir : " + bin_dir)

source_dir = cur_folder + "/" + basic_solver_folder + "/"
print("source_dir : " + source_dir)

# make a directory for binaries
try:
    os.mkdir(bin_dir)
except FileExistsError:
    print("Directory " , bin_dir,  " already exists")

with open(pcs_file_name, 'r') as pcs_file:
    for line in pcs_file:
        params_names.append(line.split()[0])
        word = line[line.find('{')+1:line.find('}')].replace(' ', '')
        params_values.append([x for x in word.split(',')])

print(params_names)
print(params_values)

params_combs = [i for i in itertools.product(*params_values)]
print('cartesian product of parameters values:')
for i in params_combs:
    print(i)
    
for params_comb in params_combs:
    comb_str = ""
    for i in range(len(params_names)):
        comb_str = comb_str + "-" + params_names[i] + "=" + params_comb[i]
    print(comb_str)
    # set name of a new directory
    dest_dir = cur_folder + "/" + basic_solver_folder + comb_str + "/"
    print("dest_dir : " + dest_dir)
    # make the directory for the current combination
    try:
        os.mkdir(dest_dir)
    except FileExistsError:
        print("Directory " , dest_dir ,  " already exists")
    # copy sources to the directory
    sys_str = "cd " + source_dir + " && cp -a * " + dest_dir
    os.system(sys_str)
    # modify the source file
    file_to_change_name = dest_dir + "sources/core/Solver.cc"
    new_lines = []
    with open(file_to_change_name, 'r') as ofile:
        for line in ofile:
            for i in range(len(params_names)):
                word = "opt_" + params_names[i]
                static_word = "static"
                if word in line and static_word in line:
                    #print(line)
                    lst = line.split(',')
                    for x in lst:
                        x_tmp = x.replace(' ','')
                        if x_tmp.isdigit():
                            line = line.replace(x_tmp, params_comb[i],1)
                            break
            new_lines.append(line)

    with open(file_to_change_name, 'w') as ofile:
        for line in new_lines:
            ofile.write(line)
    # make a binary
    sys_str = "cd " + dest_dir + " && ./starexec_build"
    print(sys_str)
    os.system(sys_str)
    new_solver_name = basic_solver_name + comb_str
    new_solver_name = new_solver_name.replace('_','-')
    sys_str = "cp " + dest_dir + "bin/" + basic_solver_name + " " + bin_dir + new_solver_name
    print(sys_str)
    os.system(sys_str)

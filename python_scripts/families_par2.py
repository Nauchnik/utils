# Oleg Zaikin, 2019
# Compare families of SAT instances by their PAR2

import zipfile
import sys
import pandas as pd
import collections
import glob, os

TIME_LIMIT = 5000.0
#zip_file_name = 'sr-2019_struct.zip'

solver_log_names = []
for file in glob.glob("*_log"):
    print(file)
    solver_log_names.append(file)
print('')
    
class family_data:
    total = 0
    solved = 0
    sat = 0
    unsat = 0
    par2 = 0.0
    def __str__(self):
        return("total : %d  solved : %d  sat : %d  unsat : %d  par2 : %.0f" % (self.total, self.solved, self.sat, self.unsat, self.par2))

def get_families_data(solver_log_name : str):
    solver_families = dict()
    solver_df = pd.read_csv(solver_log_name,sep=' ',names=['solver','cnf','time','result'])
    for index, row in solver_df.iterrows():
	#print(row['solver'])
        cnf_name = row['cnf']
        time = float(row['time'])
        result = row['result']
        if cnf_name not in cnf_folder_dict:
            print('skipping cnf_name ' + cnf_name)
            continue
        family_name = cnf_folder_dict[cnf_name]
        if family_name not in solver_families:
            f_d = family_data()
            solver_families[family_name] = f_d
        solver_families[family_name].total += 1
        if time < TIME_LIMIT and result != 'INDET':
            solver_families[family_name].solved += 1
            if result == 'SAT':
                solver_families[family_name].sat += 1
            if result == 'UNSAT':
                solver_families[family_name].unsat += 1
            solver_families[family_name].par2 += time
        elif result == 'INDET':
            solver_families[family_name].par2 += TIME_LIMIT*2
    od = collections.OrderedDict(sorted(solver_families.items()))
    with open(solver_log_name + ".csv",'w') as ofile:
        ofile.write('family total solved sat unsat par2\n')
        for key in od:
            ofile.write(key + ' ' +  str(od[key].total) + ' ' + str(od[key].solved) + ' ' + str(od[key].sat) \
             + ' ' + str(od[key].unsat) + ' ' + str(int(od[key].par2)) + '\n')
    return od


zip_file_name = ''
zip_file_name = sys.argv[1]
print('reading zip ' + zip_file_name + '\n')

zip=zipfile.ZipFile(zip_file_name)
file_names = zip.namelist()
folder_cnfs_dict = dict()
cnf_folder_dict = dict()
cnf_amount = 0
for f_name in file_names:
    if '/' in f_name:
        if f_name[-1] == '/' and f_name[0] != '.' and f_name != 'final/':
            lst = []
            if (f_name[-1] == '/'):
                new_folder_name = f_name[:-1]
            else:
                new_folder_name = f_name
            folder_cnfs_dict[new_folder_name] = lst
    if f_name.endswith(".cnf") or f_name.endswith(".cnf.bz2") or f_name.endswith(".cnf.xz"):
        mod_f_name = f_name.split('/')[-1]
        if f_name.endswith(".cnf.bz2"):
    	    cnf_name = mod_f_name[:-4]
        elif f_name.endswith(".cnf.xz"):
    	    cnf_name = mod_f_name[:-3]
        else:
    	    cnf_name = mod_f_name
        #print(cnf_name)
        for cnf_folder in folder_cnfs_dict:
            if cnf_folder in f_name:
                folder_cnfs_dict[cnf_folder].append(cnf_name)
                cnf_folder_dict[cnf_name] = cnf_folder
                cnf_amount += 1
#print(folder_cnfs_dict)
print("%d cnfs in total" % cnf_amount)

for cnf_folder in folder_cnfs_dict:
    print("%d cnfs in the folder %s" % (len(folder_cnfs_dict[cnf_folder]),cnf_folder))
print('\n')

key = next(iter(folder_cnfs_dict))
print('files in the folder ' + key + ' :')
print(folder_cnfs_dict[key])
#print(cnf_folder_dict)

for s_l_n in solver_log_names:
    print('processing solver log ' + s_l_n)
    solver_families = get_families_data(s_l_n)

# Oleg Zaikin, 2019
# Choose instances for tuning DL-parameters via SMAC

import pandas as pd
import operator
import random
import os

sample_size = 36

sat_sample_inst_names = []
unsat_sample_inst_names = []
mixed_sample_inst_names = []

class instance_info:
    status = ""
    solved = 0
    
def calc_sc(sc_log_names):
    print('\n***')
    print(sc_log_names)
    lst_df = []
    for log_name in sc_log_names:
        df = pd.read_csv(log_name, sep=' ', names=['solver', 'instance', 'time', 'status'])
        lst_df.append(df)

    instances_info = dict()
    for index, row in lst_df[0].iterrows():
        inst = instance_info()
        instances_info[row['instance']] = inst
    
    for df in lst_df:
        for index, row in df.iterrows():
            if row['status'] != 'INDET' and float(row['time']) < 5000.0:
               instances_info[row['instance']].solved = instances_info[row['instance']].solved + 1
               if instances_info[row['instance']].status == "":
                   instances_info[row['instance']].status = row['status']
    
    #for key, value in instances_info.items():
    #    print(value.status + " : "+ str(value.solved))
    
    lst = []
    for key, value in instances_info.items():
        if value.solved > 0 and value.solved < len(sc_log_names):
            lst.append((key,value))
    sat_dict = dict()
    unsat_dict = dict()
    for x in lst:
        if x[1].status == "SAT":
            sat_dict[x[0]] = x[1]
        elif x[1].status == "UNSAT":
            unsat_dict[x[0]] = x[1]
    sorted_sat = sorted(sat_dict.items(), key=lambda kv: kv[1].solved)
    half_sample_size = int(sample_size/2)
    if len(sorted_sat) > half_sample_size:
        sorted_sat = sorted_sat[:half_sample_size]
    print("\nSAT sample size : " + str(len(sorted_sat)))
    for value in sorted_sat:
        print(value[0] + " " + value[1].status + " " + str(value[1].solved))
        sat_sample_inst_names.append(value[0])
    # unsat
    sorted_unsat = sorted(unsat_dict.items(), key=lambda kv: kv[1].solved)
    if len(sorted_unsat) > half_sample_size:
        sorted_unsat = sorted_unsat[:half_sample_size]
    print("\nUNSAT sample size : " + str(len(sorted_unsat)))
    added = 0
    for value in sorted_unsat:
        print(value[0] + " " + value[1].status + " " + str(value[1].solved))
        unsat_sample_inst_names.append(value[0])
        added = added + 1
    
    if added < half_sample_size:
        mchronobt_unsat_instances = dict()
        for key, row in lst_df[0].iterrows():
            if row['status'] == 'UNSAT':
                mchronobt_unsat_instances[row['instance']] = float(row['time'])    
        sorted_mchronobt_unsat_instances = sorted(mchronobt_unsat_instances.items(), key=lambda kv: kv[1], reverse = True)
        #init_unsat_sample_inst_names = [x for x in unsat_sample_inst_names]
        #print(sorted_mchronobt_unsat_instances)
        #added = len()
        for x in sorted_mchronobt_unsat_instances:
            if x[0] in unsat_sample_inst_names:
                continue
            else:
                unsat_sample_inst_names.append(x[0])
                added = added + 1
                if added == half_sample_size:
                    break
        #print('\nmaplechronobt info on hard instances solved by lcmdist')
        #for key, row in lst_df[0].iterrows():
            #print(row['instance'])
        #    if row['instance'] in unsat_sample_inst_names and row['instance'] not in init_unsat_sample_inst_names:
        #        print(row['instance'] + " " + row['status'] + " " + str(row['time']))

sc2017_log_names = []
sc2017_log_names.append("sc2017_MapleLcmDistChronoBt")
sc2017_log_names.append("sc2017_MapleLCMDistChrBt-v3-7-2_12-3-500k")
sc2017_log_names.append("sc2017_MapleLCMDistChrBt-v3-7-2-max-dupl-learnt-size=200-max-conflicts-first-dupl=500000")
sc2017_log_names.append("sc2017_MapleLCMDistChrBt-v3-7-4")
sc2017_log_names.append("sc2017_MapleLCMDistChrBt-v3-7-2-neverclean2")
#calc_sc(sc2017_log_names)

sc2018_log_names = []
sc2018_log_names.append("sc2018_MapleLcmDistChronoBt")
sc2018_log_names.append("sc2018_MapleLCMDistChrBt-v3-7-2_12-3-500k")
sc2018_log_names.append("sc2018_MapleLCMDistChrBt-v3-7-2-max-dupl-learnt-size=200-max-conflicts-first-dupl=500000")
sc2018_log_names.append("sc2018_MapleLCMDistChrBt-v3-7-4")
sc2018_log_names.append("sc2018_MapleLCMDistChrBt-v3-7-2-neverclean2")
calc_sc(sc2018_log_names)

print("\nsat_sample_inst_names len : " + str(len(sat_sample_inst_names)))
#for x in sat_sample_inst_names:
#    print(x)
num_to_select = int(len(sat_sample_inst_names)/2)
print('num_to_select : ' + str(num_to_select))
rand_sat_inst_names = random.sample(sat_sample_inst_names, num_to_select)
#print('rand_sat_inst_names')
#print(rand_sat_inst_names)
mixed_sample_inst_names = [x for x in rand_sat_inst_names]

print("\nunsat_sample_inst_names len : " + str(len(unsat_sample_inst_names)))
#for x in unsat_sample_inst_names:
#    print(x)
rand_unsat_inst_names = random.sample(unsat_sample_inst_names, num_to_select)
#print('rand_unsat_inst_names')
#print(rand_unsat_inst_names)
for x in rand_unsat_inst_names:
    mixed_sample_inst_names.append(x)

print("\nmixed_sample_inst_names len : " + str(len(mixed_sample_inst_names)))
#for x in mixed_sample_inst_names:
#    print(x)

cur_folder = os.getcwd()
print('cur_folder : ' + cur_folder)

sat_folder = cur_folder + '/sat/'
sys_str = 'mkdir ' + sat_folder
os.system(sys_str)
for x in sat_sample_inst_names:
    sys_str = 'cd ' + cur_folder + ' && cp ' + x + ' ' + sat_folder
    os.system(sys_str)

unsat_folder = cur_folder + '/unsat/'
sys_str = 'mkdir ' + unsat_folder
os.system(sys_str)
for x in unsat_sample_inst_names:
    sys_str = 'cd ' + cur_folder + ' && cp ' + x + ' ' + unsat_folder
    os.system(sys_str)

mixed_folder = cur_folder + '/mixed/'
sys_str = 'mkdir ' + mixed_folder
os.system(sys_str)
for x in mixed_sample_inst_names:
    sys_str = 'cd ' + cur_folder + ' && cp ' + x + ' ' + mixed_folder
    os.system(sys_str)
from shutil import copyfile

inst_fname = "all_2018_rc1_reduced.csv"

with open(inst_fname, 'r') as inst_f:
    for s in inst_f:
        word = s.split(' ')[0]
        if word == "Instance":
            continue
        src = "./" + word
        dst = "../../../2019_04_SAT_Comp/testing_sc_sample_0/cnfs/" + word
        copyfile(src, dst)
        
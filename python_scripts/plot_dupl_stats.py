import os
from matplotlib.ticker import ScalarFormatter
import matplotlib.pyplot as plt

cur_folder = os.getcwd()
print("current folder " + cur_folder)

lst_max_dupl_learnt_size = []
lst_max_learnt_size = []
lst_conflicts_first_dupl = []
lst_conflicts = []

for file_name in os.listdir(cur_folder):
    if file_name.endswith(".out"):
        #print("reading " + cur_folder + '/' + file_name)
        with open(file_name, 'r') as ifile:
            for line in ifile:
                line.rstrip("\r")
                line.rstrip("\n")
                lst = line.split('c max_dupl_learnt_size ')
                if len(lst) > 1:
                    mdls = int(lst[1].replace(':',''))
                    if mdls == 0:
                        print("in file " + file_name + " max_dupl_learnt_size == 0")
                        print("skip this file")
                        break
                    lst_max_dupl_learnt_size.append(mdls)
                lst = line.split('c max_learnt_size ')
                if len(lst) > 1:
                    lst_max_learnt_size.append(int(lst[1].replace(':','')))
                lst = line.split('c conflicts_first_dupl ')
                if len(lst) > 1:
                    lst_conflicts_first_dupl.append(int(lst[1].replace(':','')))
                lst = line.split('c conflicts ')
                if len(lst) > 1:
                    #print(line)
                    word = lst[1].replace(':', '').split()[0]
                    lst_conflicts.append(int(word))
                    #print(word)

print("lst_max_dupl_learnt_size %d" % len(lst_max_dupl_learnt_size))
print(lst_max_dupl_learnt_size)

print("lst_max_learnt_size size %d" % len(lst_max_learnt_size))
print(lst_max_learnt_size)

print("lst_conflicts_first_dupl size %d" % len(lst_conflicts_first_dupl))
print(lst_conflicts_first_dupl)

print("lst_conflicts size %d" % len(lst_conflicts))
print(lst_conflicts)

values_x = []
for i in range(len(lst_max_dupl_learnt_size)):
    values_x.append(i+1)
fig, ax = plt.subplots()
ax.axis([1, 116, 1, 100])
ax.plot(values_x, lst_max_dupl_learnt_size, 'b*--', label='max_dupl_learnt_size')
ax.plot(values_x, lst_max_learnt_size, 'rx:', label='max_learnt_size')
#ax.set_xticks(values_x)
ax.set_xlabel('CNF', rotation='horizontal')
ax.set_ylabel('Learnt size', rotation='vertical')
#box = ax.get_position()
#ax.set_position([box.x0, box.y0, box.width * 0.6, box.height])
#legend = ax.legend(loc=9, fontsize='x-large', bbox_to_anchor=(1.4, 1.0), bbox_transform=ax.transAxes)
ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.1),ncol=3, fancybox=True, shadow=True)
fig.savefig('sample_178cnfs_stats.pdf', bbox_inches='tight')
#plt.show()

plt.cla()
plt.clf()

plt.boxplot(lst_max_dupl_learnt_size)
#plt.show()
fig.savefig('sample_178cnfs_stats_boxplot.pdf', bbox_inches='tight')
B=plt.boxplot(lst_max_dupl_learnt_size)
print("max : %d " % max(lst_max_dupl_learnt_size))
lst = [item.get_ydata() for item in B['whiskers']]
print(lst)
lst = [item.get_ydata() for item in B['boxes']]
print(lst)
median = [item.get_ydata() for item in B['medians']][0]
print("median : %d " % median[0])

plt.cla()
plt.clf()
print("\nconflicts: ")

plt.boxplot(lst_conflicts_first_dupl)
#plt.show()
fig.savefig('sample_178cnfs_conflicts_boxplot.pdf', bbox_inches='tight')
B=plt.boxplot(lst_conflicts_first_dupl)
print("max : %d " % max(lst_conflicts_first_dupl))
lst = [item.get_ydata() for item in B['whiskers']]
print(lst)
lst = [item.get_ydata() for item in B['boxes']]
print(lst)
median = [item.get_ydata() for item in B['medians']][0]
print("median : %d " % median[0])
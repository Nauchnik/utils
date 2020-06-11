# +---------------------------------------------------------------------------+
# | Author: Oleg Zaikin <zaikin.icc@gmail.com>                                |
# +---------------------------------------------------------------------------+

import sys
import os

MIN_REFUTED_CUBES = 1000

if len(sys.argv) < 2:
	print('Usage : prog cnf-name')
cnfname = sys.argv[1]
print('cnf : ' + cnfname)

free_vars = []

# count free variables
with open(cnfname) as cnf:
	lines = cnf.readlines()
	for line in lines:
		if line[0] == 'p' or line[0] == 'c':
			continue
		lst = line.split(' ')
		#print(line)
		for x in lst:
			if x == ' ' or x == '':
				continue
			var = abs(int(x))
			if var != 0 and var not in free_vars:
				free_vars.append(var)

print('free vars : %d' % len(free_vars))

is_exit = False
n_val = len(free_vars)
while n_val % 10 != 0:
	n_val = n_val + 1
print('n_val : %d ' % n_val)
perc_rec = 0
statname = 'stat_' + cnfname
statname = statname.replace('.','')
statname = statname.replace('/','')
ofile = open(statname,'w')
ofile.write('n cubes non-refuted-cubes refuted-cubes %-refuted-cubes time\n')
while not is_exit:
	n_val = n_val - 10
	print('n_val : %d' % n_val)
	system_str = './march_cu/march_cu ' + cnfname + ' -n ' + str(n_val) 
	o = os.popen(system_str).read()
	s = ''
	for x in o:
		s += x
	#print(s)
	lst = s.split('\n')
	# c number of cubes 2, including 0 refuted leaves
	cubes = 0
	refuted = 0
	t = 0.0
	for x in lst:
		# c time = 0.06 seconds
		if 'c time = ' in x:
			t = float(x.split('c time = ')[1].split(' ')[0])
			print('time : %.2f' % t)	
		if 'c number of cubes' in x:
			non_refuted = int(x.split('c number of cubes ')[1].split(',')[0])
			refuted = int(x.split(' refuted leaves')[0].split(' ')[-1])
			cubes = non_refuted + refuted
			print('non_refuted cubes : %d ' % non_refuted)
			print('refuted cubes : %d' % refuted)
			if cubes > 0:
				perc = refuted*100/cubes
				ofile.write('%d %d %d %d %.2f %.2f \n' % (n_val, cubes, non_refuted, refuted, perc, t))
				print('perc : %f' % perc)
				if refuted >= MIN_REFUTED_CUBES:
					if perc > perc_rec:
						perc_rec = perc
						print('new perc rec : %f' % perc_rec)
					elif perc < perc_rec and perc_rec > 0:
						is_exit = True
				if non_refuted > 1000000:
					is_exit = True
ofile.close
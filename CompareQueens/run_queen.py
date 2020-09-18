import subprocess
import time

Nmin = 1
Nmax = 19

head = 'N bt nbr  sol nds uc r2s r2u cu  minnds maxnds  ut wt st pp mm  time'

out_fname = 'out'

results = []
with open(out_fname, 'w') as o:
    o.write(head + '\n')
    for N in range(Nmin,Nmax+1):
        cmd = './Call_QueensRUCPct ' + str(N) + ' 1'
        print(cmd)
        start_time = time.time()
        res = subprocess.check_output(cmd, shell=True).decode('utf-8').splitlines()[1]
        elapsed_time = time.time() - start_time
        elapsed_time = round(elapsed_time,2)
        o.write(res + '  ' + str(elapsed_time) + '\n')
        lst = res.split()
        results.append((lst[0],lst[4],str(elapsed_time))) # (N, nodes, elapsed)

print(results)

N_name = 'N_nodes.csv'
t_name = "N_t.csv"

with open(N_name, 'w') as o:
    o.write('N nodes\n')
    for r in results:
        o.write('%s %s\n' % (r[0], r[1]))

with open(t_name, 'w') as o:
    o.write('N time\n')
    for r in results:
        o.write('%s %s\n' % (r[0], r[2]))

#!/bin/bash
echo "Random minisat launch"
echo "Usage: scriptname cnfname cpulim"
cnfname="$1"
cpulim="$2"
str=$(cksum <<< $cnfname)
read -r seed _ <<< "$str"
#echo $seed
DIR=$(dirname "${cnfname}")
#echo "..timelimit -t $cpulim -T 1../minisat_simp_SAT2013 -rnd-freq=0.05 -rnd-seed=$seed $cnfname"
cd $DIR;
../timelimit -t $cpulim -T 1 ../minisat_simp_SAT2013 -rnd-freq=0.05 -rnd-seed=$seed $cnfname;

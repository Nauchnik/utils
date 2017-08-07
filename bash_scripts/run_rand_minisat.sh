#!/bin/bash
echo "Random minisat launch"
echo "Usage: scriptname cnfname"
cnfname="$1"
str=$(cksum <<< $cnfname)
read -r seed _ <<< "$str"
#echo $seed
DIR=$(dirname "${cnfname}")
#echo "../minisat_simp_SAT2013 -rnd-freq=0.05 -rnd-seed=$seed $cnfname"
cd $DIR;
../minisat_simp_SAT2013 -rnd-freq=0.05 -rnd-seed=$seed $cnfname;

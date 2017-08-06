#!/bin/bash
echo "Random minisat launch"
echo "Usage: scriptname cnfname"
cnfname="$1"
DIR=$(dirname "${cnfname}")
cd $DIR;
../minisat_simp_SAT2013 -rnd-freq=0.05 $cnfname;

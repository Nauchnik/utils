#!/bin/bash
echo "clause decompose all cnfs in the current folder"
echo "Usage: script.sh cpulim"
cpulim="$1"
echo "cpulim " $cpulim 
clauses_filename=""
echo "Fixing CNFs"
./fix_all_cnf.sh
for cnfname in `find ./ -name "*.cnf*"`
do
    cnfname=$(basename $cnfname);
    clauses_filename=clauses_$cnfname
    ./timelimit -t $cpulim -T 1 ./maple_clausesSaving -cpu-lim=$cpulim $cnfname &> $clauses_filename;
    ./clause_decompose $clauses_filename $cnfname &> out_$cnfname;
   rm $cnfname;
   echo "file " $cnfname " processed";
done
echo "Fixing CNFs"
./fix_all_cnf.sh
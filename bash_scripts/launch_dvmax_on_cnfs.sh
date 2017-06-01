#!/bin/bash
dvmax_val="$1"
echo "launch solver on each CNF from a current folder"
for filename in `find ./ -name "*.cnf*"`
do
    filename_base=$(basename $filename);
   ./minisat_dvmax -dvmax=$dvmax_val $filename &> out_$filename_base;
   echo "file $filename_base processed"
done
#!/bin/bash
echo "Random minisat launches."
echo "Usage: scriptname cnfname timelim launches"
cnfname="$1"
timelim="$2"
launches="$3"
output=""
filename_base=""
for i in $(seq 1 $launches)
do
	filename_base=$(basename $filename);
	output=out_$filename_base"_"$i;
	./minisat -rnd-freq=0.05 -cpu-lim=$timelim ./$cnfname &> $output
done

#!/bin/bash
echo "Make copies of all CNFs in the current folder."
threads="$1"
current_cnfname=""
filename_base=""
index=0
for filename in `find ./ -name "*.cnf*"`
do
    for i in $(seq 1 $threads)
    do
	filename_base=$(basename $filename .cnf);
	index=$((i-1));
	current_cnfname=$filename_base"_"$index".cnf";
	cp ./$filename ./$current_cnfname;                    
    done
    rm ./$filename;
done
./fix_all_cnf.sh
#!/bin/bash
echo "Multithread SMAC starting."
echo "Usage: scriptname filename threads"
filename="$1"
threads="$2"
output=""
filename_base=""
export SMAC_MEMORY=2048
for i in $(seq 1 $threads)
do
    filename_base=$(basename $filename);
    #echo "filename_base" $filename_base
    output=out_$filename_base"_"$i;
    #echo "output" $output;
    nohup ./smac --scenario-file $filename --shared-model-mode true --seed $i &> $output &
    #echo "started smac # " $i;
done

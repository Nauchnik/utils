#!/bin/bash
if [ "$#" -ne 3 ]; then
    echo "Illegal number of parameters"
    echo "Usage: scriptname filename threads_number node_index"
    exit 1
fi
echo "Multithread SMAC starting."
filename="$1"
threads="$2"
node_index="$3"
output=""
filename_base=""
cur_thread_num=0
export SMAC_MEMORY=7680
for cur_thread_index in $(seq 1 $threads)
do
    cur_thread_num=$((node_index*threads+cur_thread_index)); 
    echo $cur_thread_num;
    filename_base=$(basename $filename);
    #echo "filename_base" $filename_base
    output=out_$filename_base"_"$cur_thread_num;
    echo "output" $output;
    nohup ./smac --scenario-file $filename --shared-model-mode true --seed $cur_thread_num &> $output &
    echo "started smac # " $cur_thread_num;
done
echo "SMAC_MEMORY" $SMAC_MEMORY

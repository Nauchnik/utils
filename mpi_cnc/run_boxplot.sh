#!/bin/bash
#Compute runtime estimations based on CnC results.
#
# version 0.0.1

set -o xtrace

step=$1
K=$2

hashvalues="1 0 Le rand"
simplvalues="1 10thous 100thous 1mil 10mil"

for hash in $hashvalues; do
    for simpl in $simplvalues; do
        python3 ./boxplot_solvers.py ./stat_md4_${step}steps_12Dobb_K${K}_${hash}hash-${simpl}cnflcnf -u=sample_results_md4_${step}steps_12Dobb_K${K}_${hash}hash-${simpl}cnflcnf.csv
    done
done

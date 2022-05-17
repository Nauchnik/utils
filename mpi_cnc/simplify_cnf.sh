#!/bin/bash
# Simplify a given CNF via the CaDiCaL solver.
# Use two limits of the number of clauses: 1 and 1000000.
# The first one corresponds to Unit Popagation.
#
# version 0.0.4

cnf=$1
log=simpl-$(basename $cnf)
log=${log%.*}
#
newcnf=${cnf%.*}
newcnf=$newcnf-1cnfl.cnf
cadical_1.5 -c 1 $cnf -o $newcnf > $log
echo -e "\n" >> $log
#
#newcnf=${cnf%.*}
#newcnf=$newcnf-1thouscnfl.cnf
#cadical_1.5 -c 1000 $cnf -o $newcnf >> $log
#echo -e "\n" >> $log
#
newcnf=${cnf%.*}
newcnf=$newcnf-10thouscnfl.cnf
cadical_1.5 -c 10000 $cnf -o $newcnf >> $log
echo -e "\n" >> $log
#
newcnf=${cnf%.*}
newcnf=$newcnf-100thouscnfl.cnf
cadical_1.5 -c 100000 $cnf -o $newcnf >> $log
echo -e "\n" >> $log
#
newcnf=${cnf%.*}
newcnf=$newcnf-1milcnfl.cnf
cadical_1.5 -c 1000000 $cnf -o $newcnf >> $log
echo -e "\n" >> $log
#
newcnf=${cnf%.*}
newcnf=$newcnf-10milcnfl.cnf
cadical_1.5 -c 10000000 $cnf -o $newcnf >> $log
echo -e "\n" >> $log

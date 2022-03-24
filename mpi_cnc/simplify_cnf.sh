#!/bin/bash
# Simplify a given CNF via the CaDiCaL solver.
# Use two limits of the number of clauses: 1 and 1000000.
# The first one corresponds to Unit Popagation.

cnf=$1
newcnf=${cnf%.*}
newcnf=$newcnf-1cnfl.cnf
cadical_1.4.1 -c 1 $cnf -o $newcnf -q
newcnf=${cnf%.*}
newcnf=$newcnf-1milcnfl.cnf
cadical_1.4.1 -c 1000000 $cnf -o $newcnf -q

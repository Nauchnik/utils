#!/bin/bash

version="0.0.1"

if ([ $# -ne 2 ]) then
  echo -e "script CNF CPULIM\n"
  exit 1
fi

CNF=$1
CPULIM=$2

DIR=.
SIMPLIFIER="cadical_1.4.1"
LOOKAHEAD="march_cu"

printf "CNF : %s\n" $CNF
printf "CPULIM : %d\n" $CPULIM

CNFBASE=$(basename -- "$CNF")
CNFNOEXT="${CNFBASE%.*}"
printf "CNF without extension : %s\n" $CNFNOEXT

res1=$(date +%s.%N)
mincnf=$DIR/$CNFNOEXT.mincnf
cubes=$DIR/$CNFNOEXT.cubes
formula=$DIR/$CNFNOEXT.icnf
ext=$DIR/$CNFNOEXT.ext
# Given "-c 1", cadical stops upen the first conflict, i.e. performs BCP:
$SIMPLIFIER -c 1 $CNF -o $mincnf -e $ext -q
res2=$(date +%s.%N)
elapsed=$(echo "$res2 - $res1" | bc)
elapsed=${elapsed%.*}
printf "Elapsed : %02.4f\n" $elapsed
rem=$((CPULIM-elapsed))
printf "Remaining time after simplification : %02.4f\n" $rem
if [[ $rem -le 0 ]] ; then
    exit 1
fi

timelimit -t $rem -T 1 $LOOKAHEAD $mincnf -o $cubes
res3=$(date +%s.%N)
elapsed=$(echo "$res3 - $res1" | bc)
elapsed=${elapsed%.*}
printf "elapsed : %02.4f\n" $elapsed
rem=$((CPULIM-elapsed))
printf "remaining time after cube phase : %02.4f\n" $rem
if [[ $rem -le 0 ]] ; then
    exit 1
fi

echo "p inccnf" > $formula
cat $mincnf | grep -v c >> $formula
cat $cubes >> $formula
timelimit -t $rem -T 1 $SIMPLIFIER $formula -t $rem -q

rm $mincnf $cubes $formula $ext

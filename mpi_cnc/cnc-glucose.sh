#!/bin/bash

CNF=$1
id=$2
CPULIM=$3

DIR=.
SIMPLIFIER="cadical_1.4.1"
LOOKAHEAD="march_cu"

if ([ $# -ne 3 ]) then
  echo -e "script CNF cube-id CPULIM\n"
  exit 1
fi

printf "CNF : %s\n" $CNF
printf "id : %d\n" $id
printf "CPULIM : %d\n" $CPULIM

res1=$(date +%s.%N)
mincnf=$DIR/id-$id-mincnf
cubes=$DIR/id-$id-cubes
formula=$DIR/id-$id-formula.icnf
ext=$DIR/id-$id-ext
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
timelimit -t $rem -T 1 iglucose $formula -verb=0 -cpu-lim=$rem

rm id-$id*

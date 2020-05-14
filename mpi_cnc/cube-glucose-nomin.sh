#!/bin/bash
CNF=$1
id=$2
CPULIM=$3
DIR=.
LINGTIMELIM=0
printf "id : %d\n" $id
printf "CPULIM : %d\n" $CPULIM
res1=$(date +%s.%N)
cubes=$DIR/id-$id-cubes
formula=$DIR/id-$id-formula.icnf
./timelimit -t $CPULIM -T 1 $DIR/march_cu $CNF -o $cubes
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
cat $CNF | grep -v c >> $formula
cat $cubes >> $formula
./timelimit -t $rem -T 1 $DIR/iglucose $formula -verb=0 -cpu-lim=$rem


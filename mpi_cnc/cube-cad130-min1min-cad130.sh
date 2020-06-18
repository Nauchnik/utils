#!/bin/bash
CNF=$1
id=$2
CPULIM=$3
DIR=.
SIMPTIMELIM=60
printf "id : %d\n" $id
printf "CPULIM : %d\n" $CPULIM
res1=$(date +%s.%N)
mincnf=$DIR/id-$id-mincnf
cubes=$DIR/id-$id-cubes
formula=$DIR/id-$id-formula.icnf
ext=$DIR/id-$id-ext
ext2=$DIR/id-$id-ext2
$DIR/cadical130 $CNF -o $mincnf -e $ext -t $SIMPTIMELIM -q
res2=$(date +%s.%N)
elapsed=$(echo "$res2 - $res1" | bc)
elapsed=${elapsed%.*}
printf "elapsed : %02.4f\n" $elapsed
if [[ $elapsed -lt $SIMPTIMELIM ]] ; then
    printf "solved during simplification"
    exit 1
fi 
rem=$((CPULIM-elapsed))
printf "remaining time after simplification : %02.4f\n" $rem
if [[ $rem -le 0 ]] ; then
    exit 1
fi
./timelimit -t $rem -T 1 $DIR/march_cu $mincnf -o $cubes
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
./timelimit -t $rem -T 1 $DIR/cadical130 $formula -t $rem -e $ext2

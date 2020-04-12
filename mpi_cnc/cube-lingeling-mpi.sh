CNF=$1
id=$2
CPULIM=$3
DIR=.
printf "id : %d\n" $id
printf "CPULIM : %d\n" $CPULIM
res1=$(date +%s.%N)
mincnf=$DIR/id-$id-mincnf
cubes=$DIR/id-$id-cubes
formula=$DIR/id-$id-formula.icnf
$DIR/lingeling $CNF -s -o $mincnf -T 60
res2=$(date +%s.%N)
elapsed=$(echo "$res2 - $res1" | bc)
elapsed=${elapsed%.*}
printf "elapsed : %02.4f\n" $elapsed
rem=$((CPULIM-elapsed))
printf "remaining time after minimization : %02.4f\n" $rem
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
./timelimit -t $rem -T 1 $DIR/ilingeling $formula

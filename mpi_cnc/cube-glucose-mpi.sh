CNF=$1
WUID=$2
CPULIM=$3
DIR=.
$DIR/lingeling $CNF -s -o reduced_$CNF -T 60
rm $CNF
$DIR/march_cu $DIR/reduced_$CNF -o $DIR/cubes_$WUID &> $DIR/out_march_$WUID
echo "p inccnf" > $DIR/formula_$WUID.icnf
cat reduced_$CNF | grep -v c >> $DIR/formula_$WUID.icnf
rm $DIR/reduced_$CNF
cat $DIR/cubes_$WUID >> $DIR/formula_$WUID.icnf
rm $DIR/cubes_$WUID
$DIR/iglucose $DIR/formula_$WUID.icnf -verb=0 -cpu-lim=$CPULIM
rm $DIR/formula_$WUID.icnf
rm $DIR/out_march_$WUID

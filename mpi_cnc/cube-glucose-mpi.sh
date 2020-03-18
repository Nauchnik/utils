CNF=$1
id=$2
CPULIM=$3
DIR=.
mincnf = $DIR/id-$id-mincnf
cubes = $DIR/id-$id-cubes
formula = $DIR/id-$id-formula.icnf
$DIR/lingeling $CNF -s -o $mincnf -T 60
$DIR/march_cu $mincnf -o $cubes
echo "p inccnf" > $formula
cat $mincnf | grep -v c >> $formula
cat $cubes >> $formula
$DIR/iglucose $formula -verb=0 -cpu-lim=$CPULIM

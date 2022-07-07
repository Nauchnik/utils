DIR=.
SIMPTIME=60

for file in $DIR/*.cnf
do
    echo $file
    x=$(basename $file)
    ext=ext-$x
    echo $ext
    simpcnf=simp-$SIMPTIME-sec-$x
    echo $simpcnf
    $DIR/cadical130 $file -o $simpcnf -e $ext -t $SIMPTIME -q
done

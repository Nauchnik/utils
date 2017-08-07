#!/bin/bash
for file in $directory./*.urs
do
    file=${file/'./'/}
    echo "dos2unix start on file $file"
    dos2unix ./$file
done
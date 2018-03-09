#!/bin/bash
for file in `find ./ -name "*.output*"`
do
    while IFS= read line
    do
	if echo "$line" | grep -q "true"; then
	    echo -n "1";
	elif echo "$line" | grep -q "false"; then
	    echo -n "0";
	fi
    done <"$file"
    echo " ";
done

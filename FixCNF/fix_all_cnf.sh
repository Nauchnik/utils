#!/bin/bash
echo "fixing all cnfs in the current folder"
for file in `find ./ -name "*.cnf*"`
do
   ./fix_cnf $file;
   echo "file " $file " fixed"; 
done
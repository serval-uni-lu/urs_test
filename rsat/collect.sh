#!/bin/bash

function cls {
    r=$(grep -E "^p cnf " "$1" | head -n 1)

    nbv=$(echo "$r" | cut -d ' ' -f 3)
    nbc=$(echo "$r" | cut -d ' ' -f 4)

    nbl=$(grep -v -E "^[cp]" "$1" | sed 's/ 0$//g' | grep -o -E "[-]*[0-9]*" | wc -l)

    echo "$1, $nbv, $nbc, $nbl" | sed 's/.arjun//g'
}
export -f cls

echo "file, state, mem, time" > spur.csv
cat $(find "$1" -name "*.sp") | grep -E "^/spur" | sed "s/^.*$1/$1/g" >> spur.csv

echo "file, state, mem, time" > d4.csv
cat $(find "$1" -name "*.d4") | grep -E "/d4" | sed "s/^.*$1/$1/g;s/.d4.nnf//g" >> d4.csv

echo "file, state, mem, time" > ug3.csv
cat $(find "$1" -name "*.ug3") | grep -E "^/unigen" | sed "s/^.*$1/$1/g" >> ug3.csv

echo "file, state, mem, time" > sharpSAT.csv
cat $(find "$1" -name "*.sharp") | grep -E "^/sharpSAT" | sed 's#/sharpSAT ##g' >> sharpSAT.csv

echo "file, #v, #c, #l" > cls.csv
find "$1" -name "*.cnf" | parallel -n 1 -P 6 cls >> cls.csv

echo "file, #v, #c, #l" > cls_arjun.csv
find "$1" -name "*.arjun" | parallel -n 1 -P 6 cls >> cls_arjun.csv

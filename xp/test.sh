#!/bin/bash

function run {
	r=$(./chi2.sif -c "$2" -s "$1" -a "0.0001666666666666667")
	u=$(echo "$r" | grep -E "^u " | sed 's/^u //g')
	pv=$(echo "$r" | grep -E "^pv " | sed 's/^pv //g')
	echo "$2, $u, $pv"
}
export -f run

id="1"
n="sp"

echo "file, isUniform, pvalue" > "$1_$n.csv"
find "$1" -name "*.cnf" | parallel -n 1 -P 3 run "$id" >> "$1_$n.csv"

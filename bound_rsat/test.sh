#!/bin/bash

function run {
	r=$(./chi2.sif -c "$1" -s 1 -a 0.0001666666666666667)
	u=$(echo "$r" | grep -E "^u " | sed 's/^u //g')
	pv=$(echo "$r" | grep -E "^pv " | sed 's/^pv //g')
	echo "$1, $u, $pv"
}
export -f run

echo "file, isUniform, pvalue" > "$1_sp.csv"
find "$1" -name "*.cnf" | parallel -n 1 -P 1 run >> "$1_sp.csv"

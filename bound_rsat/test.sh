#!/bin/bash


function run {
	r=$(./ug3_test.sif -c "$1")
	u=$(echo "$r" | grep -E "^u " | sed 's/^u //g')
	pv=$(echo "$r" | grep -E "^pv " | sed 's/^pv //g')
	echo "$1, $u, $pv"
}
export -f run

echo "file, isUniform, pvalue" > "$1_ug3.csv"
find "$1" -name "*.cnf" | parallel -n 1 -P 1 run >> "$1_ug3.csv"

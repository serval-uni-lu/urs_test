#!/bin/bash

function run {
	r=$(./chi2.sif -b 50 "--$2" -c "$3" -s "$1" -a "0.01")
	u=$(echo "$r" | grep -E "^is uniform " | sed 's/^u //g')
	pv=$(echo "$r" | grep -E "^hmp " | sed 's/^pv //g')
	echo "$3, $u, $pv"
}
export -f run

t="freq_var"
n="unigen3"

echo "$1_$t.$n.csv"

echo "file, isUniform, pvalue" > "$1_$t.$n.csv"
find "$1" -name "*.cnf" | parallel -n 1 -P 3 run "$n" "$t" >> "$1_$t.$n.csv"

#!/bin/bash -l
mkdir -p logs

function run {
	### d4 NO compression
	r=$(d4_mod "$1" -dDNNF -out="$1.nnf" | grep -E "^s " | sed 's/^s //g')

	echo "$1, $r"
}
export -f run


l=$(find "$1" -name "*.cnf")

echo "file, #m" > "$1_mc.csv"
echo "$l" | parallel -n 1 -j 3 run >> "$1_mc.csv"

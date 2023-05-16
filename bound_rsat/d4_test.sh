#!/bin/bash -l
mkdir -p logs

function run {
	### d4 NO compression
	r=$(d4 -mc "$1" | grep -E "^s " | sed 's/^s //g')

	echo "$1, $r"
}
export -f run


l=$(find "$1" -name "*.cnf")

echo "file, #m" > "$1_mc.csv"
echo "$l" | parallel -n 1 -j 3 run >> "$1_mc.csv"

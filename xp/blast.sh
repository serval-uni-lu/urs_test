#!/bin/bash -l
mkdir -p logs

function run {
	../blast/blast "$1" 1000 simple > "$1.tmp"
	mv "$1.tmp" "$1"

	### d4 NO compression
	r=$(d4 -mc "$1" | grep -E "^s " | sed 's/^s //g')

	echo "$1, $r"
}
export -f run


l=$(find "$1" -name "*.cnf")

echo "file, #m" > "$1_mc.csv"
echo "$l" | parallel -n 1 -j 3 run >> "$1_mc.csv"

#!/bin/bash -l
mkdir -p logs

export OMP_NUM_THREADS="2"

function run {
	r=$((d4.sif "$1" -dDNNF -out="$1.d4.nnf" && echo "ccc done") &> "$1.d4")
	r=$((spur.sif -s 1000 -no-sample-write -cnf "$1" && echo "ccc done") &> "$1.sp")
	r=$((unigen3.sif --samples 1000 "$1" && echo "ccc done") &> "$1.ug3")
	r=$((sharpSAT.sif "$1" && echo "ccc done") &> "$1.sharp")

	rm -f "$1.d4.nnf"
}
export -f run

function ar {
	arjun.sif "$1" "$1.arjun"
}
export -f ar

# tail -n 25 > 1h 43m 4.71

size="200"

l=$(find rand75 -name "*.cnf" | sort | head -n "$1" | tail -n "$size")

echo "$l" | parallel -n 1 -j 2 run
echo "$l" | parallel -n 1 -j 6 ar
l=$(find rand75 -name "*.arjun" | sort | head -n "$1" | tail -n "$size")
echo "$l" | parallel -n 1 -j 2 run
rm -rf rand75/*.txt


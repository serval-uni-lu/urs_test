#!/bin/bash -l
mkdir -p logs

export OMP_NUM_THREADS="2"

############################################################
function get_nodes {
  grep -E "^[aotfu]" "$1" | wc -l
}
export -f get_nodes

function get_edges {
  grep -E "^[0-9]" "$1" | wc -l
}
export -f get_edges

function ar {
	arjun.sif "$1" "$1.arjun"
}
export -f ar
############################################################

function run {
	### d4 NO compression
	r=$((d4.sif "$1" -dDNNF -out="$1.d4.nnf" && echo "ccc done") &> "$1.d4")

	# rn=$(get_nodes "$1.d4.nnf")
	# re=$(get_edges "$1.d4.nnf")
	# rmc=$(cat "$1.d4" | grep -E "^s " | sed 's/^s //g')

	rm -f "$1.d4.nnf"
	# echo "$1, $rn, $re, $rmc" >> d4_log

	# ### d4    compression

	r=$((d4_mod.sif "$1" -dDNNF -out="$1.d4.nnf" && echo "ccc done") &> "$1.d4_mod")

	# rn=$(get_nodes "$1.d4.nnf")
	# re=$(get_edges "$1.d4.nnf")
	# rmc=$(cat "$1.d4" | grep -E "^s " | sed 's/^s //g')

	rm -f "$1.d4.nnf"
	# echo "$1, $rn, $re, $rmc" >> d4_mod_log
}
export -f run


# tail -n 25 > 1h 43m 4.71

size="900"

l=$(find rand75 -name "*.cnf" | sort | head -n "$1" | tail -n "$size")

echo "$l" | parallel -n 1 -j 3 run
#echo "$l" | parallel -n 1 -j 6 ar
l=$(find rand75 -name "*.arjun" | sort | head -n "$1" | tail -n "$size")
echo "$l" | parallel -n 1 -j 3 run
rm -rf rand75/*.txt

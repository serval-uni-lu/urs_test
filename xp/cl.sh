#!/bin/bash

function run {
	grep -v -E "^c flip$" "$1" > tmp
	mv tmp "$1"
}
export -f run

find "$1" -name "*.cnf" | parallel -n 1 -P 1 run

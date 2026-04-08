#!/bin/bash

./splitter/build/splitter --cnf "$1" > "$1.log"
cat "$1.log" "$1" > "$1.proj"

./projection/build/projection --cnf "$1.proj"

./D4/d4/d4 -dDNNF "$1.proj.p" -out="$1.pnnf"
./D4/d4/d4 -dDNNF "$1.proj.pup" -out="$1.unnf"

rm "$1.proj"
rm "$1.proj.p"
rm "$1.proj.pup"
rm "$1.log"

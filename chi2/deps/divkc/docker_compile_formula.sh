#!/bin/bash

docker run --rm -v "$(pwd):/work:Z" -w "/work" divkc /divkc/splitter --cnf "$1" > "$1.log"
cat "$1.log" "$1" > "$1.proj"
docker run --rm -v "$(pwd):/work:Z" -w "/work" divkc /divkc/projection --cnf "$1.proj"

docker run --rm -v "$(pwd):/work:Z" -w "/work" divkc /divkc/wrap 16000 3600 /divkc/d4 -dDNNF "$1.proj.p" -out="$1.pnnf"
docker run --rm -v "$(pwd):/work:Z" -w "/work" divkc /divkc/wrap 16000 3600 /divkc/d4 -dDNNF "$1.proj.pup" -out="$1.unnf"

rm -f "$1.proj"
rm -f "$1.proj.p"
rm -f "$1.proj.pup"
rm -f "$1.log"

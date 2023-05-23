#!/bin/bash

echo "file, #m, #m_d4"
find "$1" -name "*.cnf" | parallel -n 1 -P 4 python3 "src/test.py" -f

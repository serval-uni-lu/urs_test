#!/bin/bash

cp ../BDDSampler/cudd-3.0.0/config.h ./include/config.h
cp ../BDDSampler/cudd-3.0.0/cudd/cudd.h ./include/cudd.h
cp ../BDDSampler/cudd-3.0.0/dddmp/dddmp.h ./include/dddmp.h
cp ../BDDSampler/cudd-3.0.0/util/util.h ./include/util.h

cp ../BDDSampler/cudd-3.0.0/cudd/.libs/libcudd.a ./libs/libcudd.a

g++ src/main.cpp src/CNF.cpp -o main -I./include -L./libs -lcudd

#/usr/bin/env python3

import sys
from subprocess import getoutput
import os
import uuid
import math
import shutil
import random
import argparse
import copy
import tempfile
import pandas as pd
import numpy as np
import re

from scipy.stats import chi2
from scipy.stats import chisquare

import argparse

import dDNNF

def make_temp_name(dir = tempfile.gettempdir()):
    return os.path.join(dir, str(uuid.uuid1()))

def getSolutionFromUniGen3(inputFile, numSolutions, newSeed):
    # must construct: ./approxmc3 -s 1 -v2 --sampleout /dev/null --samples 500
    inputFileSuffix = inputFile.split('/')[-1][:-4]
    # tempOutputFile = tempfile.gettempdir() + '/' + inputFileSuffix + ".txt"
    tempOutputFile = make_temp_name()

    cmd = '/unigen/build/unigen -s ' + str(int(newSeed)) + ' -v 0 --samples ' + str(numSolutions)
    cmd += ' --sampleout ' + str(tempOutputFile)
    cmd += ' ' + inputFile # + ' > /dev/null 2>&1'
    # cmd += ' ' + inputFile
    #if args.verbose:
    print("cmd: ", cmd)
    os.system(cmd)
    # os.system(f"ls \"{tempfile.gettempdir()}\"")

    with open(tempOutputFile, 'r') as f:
        lines = f.readlines()

    solList = []
    for line in lines:
        line = line.strip()
        solList.append(line)

    print(f"UNIGEN: {numSolutions} generated: {len(solList)}, seed: {newSeed}")
    solreturnList = solList
    if len(solList) > numSolutions:
        solreturnList = random.sample(solList, numSolutions)

    os.unlink(str(tempOutputFile))
    return solreturnList

def getSolutionFromSpur(inputFile, numSolutions, newSeed):
    rng = np.random.default_rng()

    inputFileSuffix = inputFile.split('/')[-1][:-4]
    # tempOutputFile = tempfile.gettempdir() + '/' + inputFileSuffix + ".out"
    tempOutputFile = make_temp_name()
    cmd = '/samplers/spur -seed %d -q -s %d -out %s -cnf %s' % (
        newSeed, numSolutions, tempOutputFile, inputFile)
    # if args.verbose:
    print("cmd: ", cmd)
    os.system(cmd)

    with open(tempOutputFile, 'r') as f:
        lines = f.readlines()

    solList = []
    startParse = False
    for line in lines:
        if (line.startswith('#START_SAMPLES')):
            startParse = True
            continue
        if (not (startParse)):
            continue
        if (line.startswith('#END_SAMPLES')):
            startParse = False
            continue
        fields = line.strip().split(',')
        solCount = int(fields[0])
        for _ in range(solCount):
            sol = ' '
            i = 1
            for x in list(fields[1]):
                if (x == '0'):
                    sol += ' -' + str(i)
                elif (x == '1'):
                    sol += ' ' + str(i)
                elif (x == '*'):
                    sol += ' ' + str(i * (rng.choice([-1, 1])))
                else:
                    print("ERROR WHILE PARSING SPUR SAMPLES")
                i += 1
            solList.append(sol)

    os.unlink(tempOutputFile)
    return solList

def getSolutionFromSTS(inputFile, numSolutions, newSeed):
    kValue = numSolutions
    samplingRounds = 1
    inputFileSuffix = inputFile.split('/')[-1][:-4]
    # outputFile = tempfile.gettempdir() + '/' + inputFileSuffix + ".out"
    outputFile = make_temp_name()
    cmd = '/STS -k=' + str(kValue) + ' -rnd-seed=' + str(newSeed) + ' -nsamples=' + str(samplingRounds) + ' ' + str(inputFile)
    cmd += ' |grep -E "^s " | sed "s/^s //g" > ' + str(outputFile)
    # if args.verbose:
    print("cmd: ", cmd)
    os.system(cmd)

    with open(outputFile, 'r') as f:
        lines = f.readlines()

    solList = []
    # baseList = {}
    for j in range(len(lines)):
        solList.append(lines[j])

    if len(solList) < numSolutions:
        print(len(solList))
        print("STS Did not find required number of solutions")
        sys.exit(1)

    if len(solList) > numSolutions:
        solreturnList = random.sample(solList, numSolutions)



    os.unlink(outputFile)
    return solList

def getSolutionFromQuickSampler(inputFile, numSolutions, newSeed):
    cmd = "/samplers/quicksampler -n " + str(numSolutions * 5) + ' ' + str(inputFile) + ' > /dev/null 2>&1'
    # if args.verbose:
    print("cmd: ", cmd)
    os.system(cmd)
    cmd = "/samplers/z3-quicksampler/z3 sat.quicksampler_check=true sat.quicksampler_check.timeout=3600.0 " + str(
        inputFile) + ' > /dev/null 2>&1'
    # os.system(cmd)

    # cmd = "/samplers/z3 "+str(inputFile)#+' > /dev/null 2>&1'
    # if args.verbose:
    print("cmd: ", cmd)
    os.system(cmd)
    if (numSolutions > 1):
        i = 0

    with open(inputFile + '.samples.valid', 'r') as f:
        validLines = f.readlines()

    solList = []
    for line in validLines:
        fields = line.strip().split(':')
        sol = fields[0][:-2]
        # print(sol)
        solList.append(sol)

    os.unlink(inputFile + '.samples')
    os.unlink(inputFile + '.samples.valid')

    if len(solList) > numSolutions:
        solList = random.sample(solList, numSolutions)

    if len(solList) != numSolutions:
        print("Did not find required number of solutions")
        sys.exit(1)

    return solList

def getSolutionFromCMSsampler(inputFile, numSolutions, newSeed):
    # inputFileSuffix = inputFile.split('/')[-1][:-4]
    # outputFile = tempfile.gettempdir() + '/' + inputFileSuffix + ".out"
    outputFile = make_temp_name()
    cmd = "/samplers/cryptominisat5 --restart luby --maple 0 --verb 10 --nobansol"
    cmd += " --scc 1 -n1 --presimp 0 --polar rnd --freq 0.9999"
    cmd += " --random " + str(int(newSeed)) + " --maxsol " + str(numSolutions)
    cmd += " " + inputFile
    cmd += " --dumpresult " + outputFile + " > /dev/null 2>&1"

    # if args.verbose:
    print("cmd: ", cmd)
    os.system(cmd)

    with open(outputFile, 'r') as f:
        lines = f.readlines()

    solList = []
    for line in lines:
        if line.strip() == 'SAT':
            continue

        sol = line.strip()[:-2]
        # print(sol)
        solList.append(sol)

    solreturnList = solList
    if len(solList) > numSolutions:
        solreturnList = random.sample(solList, numSolutions)
    if len(solList) < numSolutions:
        print("cryptominisat5 Did not find required number of solutions")
        sys.exit(1)
    os.unlink(outputFile)
    return solreturnList

def getSolutionFromLookahead(inputFile, numSolutions, newSeed):
    kValue = 50
    # samplingRounds = numSolutions / kValue + 1
    inputFileSuffix = inputFile.split('/')[-1][:-4]
    # outputFile = tempfile.gettempdir() + '/' + inputFileSuffix + ".out"
    outputFile = make_temp_name()
    cmd = '/usr/bin/python3 /lookahead.py -k ' + str(kValue) + ' -nb ' + str(numSolutions) + ' -c ' + str(inputFile)
    cmd += ' > ' + str(outputFile)
    # if args.verbose:
    print("cmd: ", cmd)
    os.system(cmd)

    with open(outputFile, 'r') as f:
        lines = f.readlines()

    solList = []
    for j in range(len(lines)):
        sol = lines[j].strip()
        solList.append(sol)

    if len(solList) < numSolutions:
        print(len(solList))
        print("Lookahead Did not find required number of solutions")
        sys.exit(1)

    if len(solList) > numSolutions:
        solreturnList = random.sample(solList, numSolutions)



    os.unlink(outputFile)
    return solList

def getSolutionFromSMARCH(inputFile, numSolutions, newSeed):
    # multi process
    # cmd = "/usr/bin/python3 /samplers/smarch_mp.py -p " + str(P_THREADS) + " -o " + os.path.dirname(inputFile) + " " + inputFile + " " + str(numSolutions) + " > /dev/null 2>&1"
    # single process

    # tmpdir = os.path.dirname(inputFile)
    tmpdir = make_temp_name()
    cmd = "/usr/bin/python3 /samplers/smarch.py -o " + tmpdir + " " + inputFile + " " + str(numSolutions) + " > /dev/null 2>&1"
    # cmd = "/usr/bin/python3 /samplers/smarch.py -o " + os.path.dirname(inputFile) + " " + inputFile + " " + str(numSolutions) + " > /dev/null 2>&1"
    # cmd = "/usr/bin/python3 /home/gilles/ICST2019-EMSE-Ext/Kclause_Smarch-local/Smarch/smarch.py " + " -o " + os.path.dirname(inputFile) + " " + inputFile + " " + str(numSolutions)
    # if args.verbose:
    print("cmd: ", cmd)
    os.system(cmd)
    # if (numSolutions > 1):
    #   i = 0
    solList = []
    # tempFile = inputFile.replace('.cnf', '_' + str(numSolutions)) + '.samples'
    tempFile = tmpdir + "/" + os.path.basename(inputFile).replace('.cnf', "_" + str(numSolutions)) + ".samples"
    # if args.verbose:
    print(tempFile)

    df = pd.read_csv(tempFile, header=None)

    # with open(inputFile+'.samples', 'r') as f:
    #   lines = f.readlines()
    for x in df.values:
        # tmpLst = []
        lst = x.tolist()
        sol = ''
        for i in lst:
            if not math.isnan(i):
                sol += ' ' + str(int(i))
        solList.append(sol)
        # tmpList = [str(int(i)) for i in lst if not math.isnan(i)]
    # if args.verbose:
    #    print(sol)

    # solList.append(tmpList)
    # solList = [x for x in df.values]
    os.unlink(tempFile)
    # os.unlink(tmpdir)
    shutil.rmtree(tmpdir)

    return solList

def getSolutionFromKUS(inputFile, numSolutions, newSeed):

    inputFileSuffix = inputFile.split('/')[-1][:-4]
    # tempOutputFile = tempfile.gettempdir() + '/' + inputFileSuffix + ".txt"
    tempOutputFile = make_temp_name()
    cwd = os.getcwd()
    cmd = '/usr/bin/python3  /samplers/KUS.py --samples=' + str(numSolutions) + ' ' + '--outputfile ' + tempOutputFile
    cmd += ' ' + str(os.path.abspath(inputFile)) + ' > /dev/null 2>&1'
    # if args.verbose:
    print("cmd: ", cmd)
    # os.chdir(str(os.getcwd()) + '/samplers')
    os.system(cmd)
    # os.chdir(str(cwd))

    with open(tempOutputFile, 'r') as f:
        lines = f.readlines()

    solList = []

    for line in lines:
        sol = re.sub('[0-9]*,', '', line)
        sol = sol.strip()
        solList.append(sol)

    os.unlink(str(tempOutputFile))

    return solList



def get_mc(cnf):
    D4_cmd = '/d4 -mc \"{}\" 2>&1 | grep -E \'^s [0-9]+$\' | sed \'s/^s //g\''
    r = getoutput(D4_cmd.format(cnf))
    return int(r)

def compute_dDNNF(cnf):
    tmp = make_temp_name()
    D4_cmd = '/d4 \"{}\" -dDNNF -out=\"{}\" 2>&1 | grep -E \'^s [0-9]+$\' | sed \'s/^s //g\''
    r = getoutput(D4_cmd.format(cnf, tmp))
    return tmp



def count_repeats(samples, sample_size):
    d = set()

    nb = 0
    i = 0
    for line in samples:
        i += 1
        if line in d:
            nb += 1
        d.add(line)
        if i >= sample_size:
            break
    return nb

def make_bins(samples, sample_size):
    d = dict()

    i = 0
    for s in samples:
        i += 1
        if s in d:
            d[s] += 1
        else:
            d[s] = 1
        if i >= sample_size:
            break

    res = []
    for s in d:
        res.append(d[s])
    return res

def monobit():
    total_mc = nnf.get_node(1).mc

    even = 0
    for i in range(0, len(nnf.get_node(1).mc_by_nb_features)):
        if i % 2 == 0:
            even += nnf.get_node(1).mc_by_nb_features[i]

    uneven = total_mc - even

    sample_size = max(batch_size, math.ceil(total_mc * min_elem_per_cell / min(uneven, even)))

    for _ in range(0, args.n):
        samples = []
        while len(samples) < sample_size:
             samples.extend(sampler_fn(cnf_file, batch_size, random.randint(0, 2**31 - 1)))
             print("##############################")
             print(len(samples))
             print("##############################")

        # building the ovserved = [even, uneven] array
        # if nb features is even then modulo 2 becomes 0 thus the index in the array
        # similarily for an uneven nb features

        observed = [0, 0]
        for s in samples:
            n = 0
            for f in s.strip().split(" "):
                if int(f) > 0:
                    n += 1

            observed[n % 2] += 1

        expected = [even * len(samples) / total_mc, uneven * len(samples) / total_mc]

        X2, pv = chisquare(observed, expected)
        crit = chi2.ppf(1 - significance_level, df = len(observed) - 1)
        print(f"X2 {X2}")
        print(f"crit {crit}")
        print(f"pv {pv}")
        # print(f"u {X2 <= crit}")
        print(f"is uniform {pv >= significance_level}")
        # print(f"X2: {X2} ({pv})\ncrit: {crit}")
        # print(X2 <= crit)



parser = argparse.ArgumentParser()
parser.add_argument("-c", "--cnf", type=str, help="path to the cnf formula")
# parser.add_argument("-k", type=int, default=50)
parser.add_argument("-a", type=float, default=0.05, help="set the significance level")
parser.add_argument("-n", type=int, default=1, help="number of times to repeat the test")
parser.add_argument("-b", "--batch_size", type=int, default=20, help="set the batch size, i.e. the number of solutions asked to the sampler")
parser.add_argument("-s", "--sampler", type=str, default="unigen3", help="set the sampler to test")

parser.add_argument("--min_elem_per_cell", type=int, default=5, help="set the minimum expected elements per cell for chi-squared tests")

parser.add_argument("--monobit", type=bool, const=True, nargs='?', default=False, help="if set then the monobit test will be executed")

UNIGEN3 = "unigen3"
SPUR = "spur"
STS = "sts"
SMARCH = "smarch"
LOOKAHEAD = "lookahead"
QUICKSAMPLER = "quicksampler"
CMSGEN = "cmsgen"
KUS = "kus"

args = parser.parse_args()


significance_level = args.a
cnf_file = args.cnf
batch_size = args.batch_size
min_elem_per_cell = args.min_elem_per_cell

sampler_fn = getSolutionFromUniGen3

args.sampler = args.sampler.lower()

if args.sampler == UNIGEN3:
    sampler_fn = getSolutionFromUniGen3
elif args.sampler == SPUR:
    sampler_fn = getSolutionFromSpur
elif args.sampler == STS:
    sampler_fn = getSolutionFromSTS
elif args.sampler == SMARCH:
    sampler_fn = getSolutionFromSMARCH
elif args.sampler == LOOKAHEAD:
    sampler_fn = getSolutionFromLookahead
elif args.sampler == QUICKSAMPLER:
    sampler_fn = getSolutionFromQuickSampler
elif args.sampler == CMSGEN:
    sampler_fn = getSolutionFromCMSsampler
elif args.sampler == KUS:
    sampler_fn = getSolutionFromKUS

dDNNF_path = compute_dDNNF(cnf_file)

nnf = dDNNF.from_file(dDNNF_path)
nnf.annotate_mc()

if args.monobit:
    monobit()

os.unlink(dDNNF_path)

## Chi2 test
# rng_range = get_mc(cnf_file)
# expected = [5] * rng_range
# sample_size = 5 * rng_range
# 
# print(f"sample size: {sample_size}")
# 
# for _ in range(0, args.n):
#     samples = []
#     while len(samples) < sample_size:
#         # samples = getSolutionFromUniGen3(cnf_file, batch_size, random.randint(0, 2**32 - 1))
#         # samples = getSolutionFromSTS(cnf_file, batch_size, random.randint(0, 2**32 - 1))
#         # samples.extend(getSolutionFromSpur(cnf_file, batch_size, random.randint(0, 2**31 - 1)))
#         samples.extend(sampler_fn(cnf_file, batch_size, random.randint(0, 2**31 - 1)))
#         print("##############################")
#         print(len(samples))
#         print("##############################")
# 
#     observed = make_bins(samples, sample_size)
# 
#     while len(observed) < rng_range:
#         observed.append(0)
#     print(observed)
# 
#     X2, pv = chisquare(observed, expected)
#     crit = chi2.ppf(1 - significance_level, df = rng_range - 1)
#     print(f"X2 {X2}")
#     print(f"crit {crit}")
#     print(f"pv {pv}")
#     # print(f"u {X2 <= crit}")
#     print(f"u {pv >= significance_level}")
#     # print(f"X2: {X2} ({pv})\ncrit: {crit}")
#     # print(X2 <= crit)

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
import time

from scipy.stats import chi2
from scipy.stats import chisquare

import argparse

import dDNNF
import DIMACS

def make_temp_name(dir = tempfile.gettempdir()):
    return os.path.join(dir, str(uuid.uuid1()))


# We need a dictionary for Distribution-aware distance sampling
# which records names and not feature ids in outputted samples
features_dict = {}

def create_features_dict(inputFile):
    nb_vars = 0
    with open(inputFile, 'r') as f:
        lines = f.readlines()
    for line in lines:
        if line.startswith("c") and not line.startswith("c ind"):
            line = line[0:len(line) - 1]
            _feature = line.split(" ", 4)
            del _feature[0]
            # handling non-numeric feature IDs, necessary to parse os-like models with $ in feature names...
            if len(_feature) <= 2 and len(_feature) > 0:  # needs to deal with literate comments, e.g., in V15 models
                if (_feature[0].isdigit()):
                    _feature[0] = int(_feature[0])
                else:
                    # num_filter = filter(_feature[0].isdigit(), _feature[0])
                    num_feature = "".join(c for c in _feature[0] if c.isdigit())
                    _feature[0] = int(num_feature)
                    # print('key ' +  str(_feature[1]) +  ' value ' + str(_feature[0])) -- debug
                    global features_dict
                features_dict.update({str(_feature[1]): str(_feature[0])})
        elif line.startswith('p cnf'):
            _line = line.split(" ", 4)
            nb_vars = int(_line[2])
            print("there are : " + str(nb_vars) + " integer variables")
    if (len(features_dict.keys()) == 0):
        print("could not create dict from comments, faking it with integer variables in the 'p cnf' header")
        for i in range(1, nb_vars + 1):
            # global features_dict
            features_dict.update({str(i): str(i)})

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
                    sol += ' ' + str(i * (random.choice([-1, 1])))
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
        # sys.exit(1)

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
        # sys.exit(1)

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
        sol = " ".join(set(sol.strip().split(" ")))
        solList.append(sol)

    os.unlink(str(tempOutputFile))

    return solList

def getSolutionFromDistAware(inputFile, numSolutions, newSeed):

    inputFileSuffix = inputFile.split('/')[-1][:-4]
    # tempOutputFile = tempfile.gettempdir() + '/' + inputFileSuffix + ".txt"
    tempOutputFile = make_temp_name() + ".txt"

    # creating the file to configure the sampler
    # dbsConfigFile = tempfile.gettempdir() + '/' + inputFileSuffix + ".a"
    dbsConfigFile = make_temp_name() + ".a"

    logFile = make_temp_name() + ".txt"
    # logFile = str(tempfile.gettempdir()) + '/' + "output.txt"

    with open(dbsConfigFile, 'w+') as f:
        f.write("log " + logFile + "\n")
        f.write("dimacs " + str(os.path.abspath(inputFile)) + "\n")
        params = " solver z3" + "\n"
        params += "hybrid distribution-aware distance-metric:manhattan distribution:uniform onlyBinary:true onlyNumeric:false"
        params += " selection:SolverSelection number-weight-optimization:1"
        params += " numConfigs:" + str(numSolutions)
        f.write(params + "\n")
        f.write("printconfigs " + tempOutputFile)

    cmd = "mono /samplers/distribution-aware/CommandLine.exe "
    cmd += dbsConfigFile

    # if args.verbose:
    print("cmd: ", cmd)
    os.system(cmd)

    with open(tempOutputFile, 'r') as f:
        lines = f.readlines()

    solList = []

    for line in lines:
        features = re.findall("%\w+%", line)
        sol = []

        for feature in features:
            feat = feature[1:-1]
            sol.append(feat)

        solution = ''

        for k, v in features_dict.items():
            if k in sol:
                solution += ' ' + str(v)
            else:
                solution += ' -' + str(v)
        solution = solution.strip()
        solList.append(solution)

    # cleaning temporary files
    os.unlink(str(tempOutputFile))
    os.unlink(dbsConfigFile)
    os.unlink(logFile)
    # os.unlink(str(tempfile.gettempdir()) + '/' + "output.txt_error")
    os.unlink(logFile + "_error")

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

def monobit():
    total_mc = nnf.get_node(1).mc

    even = 0
    for i in range(0, len(nnf.get_node(1).mc_by_nb_vars)):
        if i % 2 == 0:
            even += nnf.get_node(1).mc_by_nb_vars[i]

    uneven = total_mc - even

    if uneven == 0 or even == 0:
        print("pv 1")
        print("is uniform True")
        print("timeout even/uneven is zero")
        return

    sample_size = max(batch_size, math.ceil(total_mc * min_elem_per_cell / min(uneven, even)))

    for _ in range(0, args.n):
        observed = [0, 0]
        nb_samples = 0
        while nb_samples < sample_size:
            samples = sampler_fn(cnf_file, batch_size, random.randint(0, 2**31 - 1))
            nb_samples += len(samples)

            print(str(nb_samples) + " / " + str(sample_size))

            for s in samples:
                n = 0
                for f in s.strip().split(" "):
                    if int(f) > 0:
                        n += 1

                observed[n % 2] += 1

            if max_end_time <= time.time():
                print("timeout True")
                return

        # building the ovserved = [even, uneven] array
        # if nb features is even then modulo 2 becomes 0 thus the index in the array
        # similarily for an uneven nb features

        expected = [even * nb_samples / total_mc, uneven * nb_samples / total_mc]

        X2, pv = chisquare(observed, expected, ddof = 0)
        crit = chi2.ppf(1 - significance_level, df = len(observed) - 1)

        if pv <= 0:
            pv = sys.float_info.min
        if math.isnan(pv):
            pv = 1

        print(f"X2 {X2}")
        print(f"crit {crit}")
        print(f"pv {pv}")
        # print(f"u {X2 <= crit}")
        print(f"is uniform {pv > significance_level}")
        print("timeout False")
        # print(f"is uniform {pv > significance_level and pv < 1 - significance_level}")
        # print(f"X2: {X2} ({pv})\ncrit: {crit}")
        # print(X2 <= crit)

def frequency_variables():
    total_mc = nnf.get_node(1).mc
    expected = nnf.get_node(1).mc_by_var

    m = nnf.get_node(1).mc
    for i in expected:
        if expected[i] != 0 and expected[i] != total_mc:
            m = min(m, nnf.get_node(1).mc_by_var[i])

    sample_size = max(batch_size, math.ceil(total_mc * min_elem_per_cell / m))

    for _ in range(0, args.n):
        observed = {}
        r_expected = {}
        nb_tested_vars = 0
        for i in expected:
            if expected[i] != 0 and expected[i] != total_mc:
                observed[i] = 0
                #r_expected[i] = expected[i] * len(samples) / total_mc
                nb_tested_vars += 1

        nb_samples = 0
        while nb_samples < sample_size:
            samples = sampler_fn(cnf_file, batch_size, random.randint(0, 2**31 - 1))
            nb_samples += len(samples)


            for s in samples:
                for f in s.strip().split(" "):
                    l = int(f)
                    if l > 0 and expected[abs(l)] != 0 and expected[abs(l)] != total_mc:
                        observed[abs(l)] += 1

            if max_end_time <= time.time():
                print("timeout True")
                return


        for i in expected:
            if expected[i] != 0 and expected[i] != total_mc:
                r_expected[i] = expected[i] * nb_samples / total_mc

        if nb_tested_vars > 0:
            is_uniform = True
            pvs = []
            for i in expected:
                if expected[i] != 0 and expected[i] != total_mc:
                    r_observed = [observed[i], nb_samples - observed[i]]

                    e = expected[i] * nb_samples / total_mc
                    r_expected = [e, nb_samples - e]

                    print(f"obs: {r_observed}")
                    print(f"exp: {r_expected}")

                    X2, pv = chisquare(r_observed, r_expected, ddof = 0)
                    crit = chi2.ppf(1 - (significance_level / nb_tested_vars), df = len(r_observed) - 1)

                    if pv <= 0:
                        pv = sys.float_info.min
                    if math.isnan(pv):
                        pv = 1

                    print(f"v{i} X2 {X2}")
                    print(f"v{i} crit {crit}")
                    print(f"v{i} pv {pv}")
                    # print(f"u {X2 <= crit}")
                    pvs.append(pv)
                    is_uniform = is_uniform and pv > (significance_level / nb_tested_vars)
                    print(f"v{i} is uniform {pv > (significance_level / nb_tested_vars)}")
                    # print(f"is uniform {pv > significance_level and pv < 1 - significance_level}")
                    # print(f"X2: {X2} ({pv})\ncrit: {crit}")
                    # print(X2 <= crit)

            pr = 1.0 / ((1.0 / nb_tested_vars) * (np.sum(1.0 / np.array(pvs))))
            if pr <= 0:
                pr = sys.float_info.min
            if math.isnan(pr):
                pr = 1

            print(f"pv {pr}")
            print(f"is uniform {is_uniform}")
            print("timeout False")
        else:
            print(f"pv {1}")
            print(f"is uniform {True}")
            print("timeout no vars to test")


def frequency_nb_variables():
    total_mc = nnf.get_node(1).mc
    expected = nnf.get_node(1).mc_by_nb_vars

    m = nnf.get_node(1).mc
    for i in expected:
        if i != 0:
            m = min(m, i)

    sample_size = max(batch_size, math.ceil(total_mc * min_elem_per_cell / m))

    for _ in range(0, args.n):
        nb_samples = 0
        observed = [0] * len(expected)

        while nb_samples < sample_size:
            samples = sampler_fn(cnf_file, batch_size, random.randint(0, 2**31 - 1))
            nb_samples += len(samples)

            print(str(nb_samples) + " / " + str(sample_size))

            for s in samples:
                n = 0
                for f in s.strip().split(" "):
                    if int(f) > 0:
                        n += 1
                observed[n] += 1

            if max_end_time <= time.time():
                print("timeout True")
                return


        for s in samples:
            n = 0
            for f in s.strip().split(" "):
                if int(f) > 0:
                    n += 1
            observed[n] += 1

        r_observed = []
        r_expected = []
        for i in range(0, len(expected)):
            if expected[i] != 0:
                r_observed.append(observed[i])
                r_expected.append(expected[i] * nb_samples / total_mc)

        X2, pv = chisquare(r_observed, r_expected, ddof = 0)
        crit = chi2.ppf(1 - significance_level, df = len(r_observed) - 1)

        if pv <= 0:
            pv = sys.float_info.min
        if math.isnan(pv):
            pv = 1

        print(f"X2 {X2}")
        print(f"crit {crit}")
        print(f"pv {pv}")
        # print(f"u {X2 <= crit}")
        print(f"is uniform {pv > significance_level}")
        print("timeout False")
        # print(f"is uniform {pv > significance_level and pv < 1 - significance_level}")
        # print(f"X2: {X2} ({pv})\ncrit: {crit}")
        # print(X2 <= crit)

def bday_repetition_pval(samples, sample_size, rng_range):
    d = set()

    nb = 0
    for line in samples:
        nb += 1
        if line in d:
            pv = 1 - (math.factorial(rng_range) / (math.factorial(rng_range - nb) * (rng_range**nb)))
            return pv
        d.add(line)


def birthday_test():
    # implementation of:
    # https://www.pcg-random.org/posts/birthday-test.html

    desired = args.bday_prob
    factor = 0
    if desired < 1.0:
        factor = math.sqrt(-2.01 * math.log(desired))
    else:
        factor = math.sqrt(2.01 * desired)

    rng_range = nnf.get_node(1).mc
    sample_size = math.ceil(factor * math.sqrt(rng_range))
    expected = sample_size - rng_range * -1 * math.expm1(sample_size * math.log1p(-1 / rng_range))
    p_zero = math.exp(-1 * expected)

    print(f"sample size: {sample_size} ({factor})")
    print(f"expected: {expected}")
    print(f"prob of zero duplicates: {p_zero}")

    for _ in range(0, args.n):
        samples = []
        while len(samples) < sample_size:
            samples.extend(sampler_fn(cnf_file, batch_size, random.randint(0, 2**31 - 1)))

            print(str(len(samples)) + " / " + str(sample_size))

            if max_end_time <= time.time():
                print("timeout True")
                return

        repeats = count_repeats(samples, sample_size)

        print(f"expected repeats: {expected}")
        print(f"observed repeats: {repeats}")

        gof_e = [expected, sample_size - expected]
        gof_o = [repeats, sample_size - repeats]

        X2, pv = chisquare(gof_o, gof_e, ddof = 0)
        crit = chi2.ppf(1 - significance_level, df = len(gof_e) - 1)

        if pv <= 0:
            pv = sys.float_info.min
        if math.isnan(pv):
            pv = 1

        print(f"X2 {X2}")
        print(f"crit {crit}")
        print(f"pv {pv}")
        # print(f"u {X2 <= crit}")
        print(f"is uniform {pv > significance_level}")
        print("timeout False")

        p_value = 0
        for k in range(0, repeats + 1):
            pdf_value = math.exp(math.log(expected) * k - expected - math.lgamma(1.01 + k))
            p_value += pdf_value
            if p_value > 0.5:
                p_value = p_value - 1

        if p_value < 0:
            # print(f"1 - {-1 * p_value}")
            p_value = 1 - (-1 * p_value)
        # else:
            # print(f"{p_value}")

        # vnr = math.perm(rng_range, sample_size)
        # vt = rng_range**sample_size

        # print(f"pvi {bday_repetition_pval(samples, sample_size, rng_range)}")

        # print(f"pv {p_value}")
        # print(f"pvb {1 - (vnr / vt)}")
        # print(f"is uniform {p_value > significance_level and p_value < 1 - significance_level}")
        # print(f"is uniform {pv > significance_level}")

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


def pearson_chisquared():
    rng_range = nnf.get_node(1).mc
    expected = [min_elem_per_cell] * rng_range
    sample_size = rng_range * min_elem_per_cell

    print(f"sample size: {sample_size}")

    for _ in range(0, args.n):
        #samples = []
        nb_samples = 0
        bins = dict()
        while nb_samples < sample_size:
            # samples = getSolutionFromUniGen3(cnf_file, batch_size, random.randint(0, 2**32 - 1))
            # samples = getSolutionFromSTS(cnf_file, batch_size, random.randint(0, 2**32 - 1))
            # samples.extend(getSolutionFromSpur(cnf_file, batch_size, random.randint(0, 2**31 - 1)))
            samples = sampler_fn(cnf_file, batch_size, random.randint(0, 2**31 - 1))
            for s in samples:
                nb_samples += 1
                if s in bins:
                    bins[s] += 1
                else:
                    bins[s] = 1
                if nb_samples >= sample_size:
                    break
                
            print(str(nb_samples) + " / " + str(sample_size))

            if max_end_time <= time.time():
                print("timeout True")
                return

        #observed = make_bins(samples, sample_size)

        observed = []
        for s in bins:
            observed.append(bins[s])

        while len(observed) < rng_range:
            observed.append(0)
        # print(observed)

        X2, pv = chisquare(observed, expected, ddof = 0)
        crit = chi2.ppf(1 - significance_level, df = rng_range - 1)

        if pv <= 0:
            pv = sys.float_info.min
        if math.isnan(pv):
            pv = 1

        print(f"X2 {X2}")
        print(f"crit {crit}")
        print(f"pv {pv}")
        # print(f"u {X2 <= crit}")
        print(f"is uniform {pv > significance_level}")
        print("timeout False")
        # print(f"is uniform {pv > significance_level and pv < 1 - significance_level}")
        # print(f"X2: {X2} ({pv})\ncrit: {crit}")
        # print(X2 <= crit)


parser = argparse.ArgumentParser()
parser.add_argument("-c", "--cnf", type=str, help="path to the cnf formula")
# parser.add_argument("-k", type=int, default=50)
parser.add_argument("-a", type=float, default=0.05, help="set the significance level")
parser.add_argument("-n", type=int, default=1, help="number of times to repeat the test")
parser.add_argument("-b", "--batch_size", type=int, default=20, help="set the batch size, i.e. the number of solutions asked to the sampler. If it is k<0 then it is converted to abs(k) * #models of formula")
parser.add_argument("-s", "--sampler", type=str, default="unigen3", help="set the sampler to test")

parser.add_argument("-t", "--timeout", type=int, default="18000", help="sets the timeout for the test, if exceeded the test is cancelled")

parser.add_argument("--min_elem_per_cell", type=int, default=10, help="set the minimum expected elements per cell for chi-squared tests")
parser.add_argument("--bday_prob", type=float, default=10, help="set the desired probability for the birthday test if v < 1.0, otherwise it sets the expected number of repeats")

parser.add_argument("--monobit", type=bool, const=True, nargs='?', default=False, help="if set then the monobit test will be executed")
parser.add_argument("--freq_var", type=bool, const=True, nargs='?', default=False, help="if set then the var frequency test will be executed")
parser.add_argument("--freq_nb_var", type=bool, const=True, nargs='?', default=False, help="if set then the number of selected var frequency test will be executed")
parser.add_argument("--bday", type=bool, const=True, nargs='?', default=False, help="if set then the birthday test will be executed")
parser.add_argument("--chisquared", type=bool, const=True, nargs='?', default=False, help="if set then Pearson's chi-squared test will be executed")

UNIGEN3 = "unigen3"
SPUR = "spur"
STS = "sts"
SMARCH = "smarch"
LOOKAHEAD = "lookahead"
QUICKSAMPLER = "quicksampler"
CMSGEN = "cmsgen"
KUS = "kus"
DISTAWARE = "distaware"

args = parser.parse_args()


significance_level = args.a
cnf_file = args.cnf
batch_size = args.batch_size
min_elem_per_cell = args.min_elem_per_cell

max_time = args.timeout

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
elif args.sampler == DISTAWARE:
    sampler_fn = getSolutionFromDistAware
    create_features_dict(cnf_file)

start_time = time.time()
max_end_time = time.time() + max_time

dimacs = DIMACS.from_file(cnf_file)
dDNNF_path = compute_dDNNF(cnf_file)

nnf = dDNNF.from_file(dDNNF_path)
nnf.annotate_mc()

if batch_size < 0:
    batch_size = abs(batch_size) * nnf.get_node(1).mc

print(f"batch size: {batch_size}")

if args.monobit:
    monobit()
if args.freq_var:
    frequency_variables()
if args.freq_nb_var:
    frequency_nb_variables()
if args.bday:
    birthday_test()
if args.chisquared:
    pearson_chisquared()

os.unlink(dDNNF_path)

exec_time = time.time() - start_time
print(f"total time: {exec_time}")

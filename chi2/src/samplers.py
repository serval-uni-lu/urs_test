import util
import os
import sys
import math
import pandas as pd
import shutil
import re
import time
import random

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
    tempOutputFile = util.make_temp_name()

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
    return list(map(util.solstr_to_frozenset, solreturnList))

def getSolutionFromSpur(inputFile, numSolutions, newSeed):
    inputFileSuffix = inputFile.split('/')[-1][:-4]
    # tempOutputFile = tempfile.gettempdir() + '/' + inputFileSuffix + ".out"
    tempOutputFile = util.make_temp_name()
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

    solreturnList = solList
    if len(solList) > numSolutions:
        solreturnList = random.sample(solList, numSolutions)
    return list(map(util.solstr_to_frozenset, solreturnList))

def getSolutionFromSTS(inputFile, numSolutions, newSeed):
    kValue = numSolutions
    samplingRounds = 1
    inputFileSuffix = inputFile.split('/')[-1][:-4]
    # outputFile = tempfile.gettempdir() + '/' + inputFileSuffix + ".out"
    outputFile = util.make_temp_name()
    cmd = '/deps/STS/core/STS -k=' + str(kValue) + ' -rnd-seed=' + str(newSeed) + ' -nsamples=' + str(samplingRounds) + ' ' + str(inputFile)
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

    if len(solList) <= 0:
        print(len(solList))
        print("STS Did not find solutions")
        sys.exit(1)

    os.unlink(outputFile)

    solreturnList = solList
    if len(solList) > numSolutions:
        solreturnList = random.sample(solList, numSolutions)
    return list(map(util.solstr_to_frozenset, solreturnList))

def getSolutionFromSTSsingle(inputFile, numSolutions, newSeed):
    kValue = 50
    samplingRounds = numSolutions
    inputFileSuffix = inputFile.split('/')[-1][:-4]
    # outputFile = tempfile.gettempdir() + '/' + inputFileSuffix + ".out"
    outputFile = util.make_temp_name()
    cmd = '/deps/STS/core/STS -k=' + str(kValue) + ' -rnd-seed=' + str(newSeed) + ' -nsamples=' + str(samplingRounds) + ' ' + str(inputFile)
    cmd += ' > ' + str(outputFile)
    # if args.verbose:
    print("cmd: ", cmd)
    os.system(cmd)

    tmpl = []
    solList = []
    with open(outputFile, 'r') as f:
        for line in f:
            if line.startswith('s '):
                tmpl.append(line[1:].strip())
            elif len(tmpl) > 0:
                solList.append(random.choice(tmpl))
                tmpl = []

    if len(solList) <= 0:
        print(len(solList))
        print("STS single Did not find solutions")
        sys.exit(1)

    os.unlink(outputFile)

    solreturnList = solList
    if len(solList) > numSolutions:
        solreturnList = random.sample(solList, numSolutions)
    return list(map(util.solstr_to_frozenset, solreturnList))

def getSolutionFromQuickSampler(inputFile, numSolutions, newSeed):
    cmd = "/deps/quicksampler/quicksampler -s " + str(newSeed) + " -n " + str(numSolutions * 5) + ' ' + str(inputFile) + ' > /dev/null 2>&1'
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

    if len(solList) <= 0:
        print("Did not find solutions")
        sys.exit(1)

    solreturnList = solList
    if len(solList) > numSolutions:
        solreturnList = random.sample(solList, numSolutions)
    return list(map(util.solstr_to_frozenset, solreturnList))

def getSolutionFromCMSsampler(inputFile, numSolutions, newSeed):
    # inputFileSuffix = inputFile.split('/')[-1][:-4]
    # outputFile = tempfile.gettempdir() + '/' + inputFileSuffix + ".out"
    outputFile = util.make_temp_name()
    # cmd = "/samplers/cryptominisat5 --restart luby --maple 0 --verb 10 --nobansol"
    # cmd += " --scc 1 -n1 --presimp 0 --polar rnd --freq 0.9999"
    # cmd += " --random " + str(int(newSeed)) + " --maxsol " + str(numSolutions)
    # cmd += " " + inputFile
    # cmd += " --dumpresult " + outputFile + " > /dev/null 2>&1"

    # cmd = f"/cmsgen/build/cmsgen --fixedconfl 3000 --seed {newSeed} --samples {numSolutions} --samplefile \"{outputFile}\" \"{inputFile}\" > /dev/null 2>&1"
    cmd = f"/cmsgen/build/cmsgen --seed {newSeed} --samples {numSolutions} --samplefile \"{outputFile}\" \"{inputFile}\" > /dev/null 2>&1"

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
    if len(solList) <= 0:
        print("cryptominisat5 Did not find solutions")
        sys.exit(1)
    os.unlink(outputFile)
    return list(map(util.solstr_to_frozenset, solreturnList))

def getSolutionFromLookahead(inputFile, numSolutions, newSeed):
    kValue = 50
    # samplingRounds = numSolutions / kValue + 1
    inputFileSuffix = inputFile.split('/')[-1][:-4]
    # outputFile = tempfile.gettempdir() + '/' + inputFileSuffix + ".out"
    outputFile = util.make_temp_name()
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

    if len(solList) <= 0:
        print(len(solList))
        print("Lookahead Did not find solutions")
        sys.exit(1)

    solreturnList = solList
    if len(solList) > numSolutions:
        solreturnList = random.sample(solList, numSolutions)



    os.unlink(outputFile)
    return list(map(util.solstr_to_frozenset, solreturnList))

def getSolutionFromSMARCH(inputFile, numSolutions, newSeed):
    # multi process
    # cmd = "/usr/bin/python3 /samplers/smarch_mp.py -p " + str(P_THREADS) + " -o " + os.path.dirname(inputFile) + " " + inputFile + " " + str(numSolutions) + " > /dev/null 2>&1"
    # single process

    # tmpdir = os.path.dirname(inputFile)
    tmpdir = util.make_temp_name()
    cmd = "/usr/bin/python3 /samplers/smarch.py -o " + tmpdir + " " + inputFile + " " + str(numSolutions) #+ " > /dev/null 2>&1"
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

    solreturnList = solList
    if len(solList) > numSolutions:
        solreturnList = random.sample(solList, numSolutions)
    return list(map(util.solstr_to_frozenset, solreturnList))

def getSolutionFromKUS(inputFile, numSolutions, newSeed):

    inputFileSuffix = inputFile.split('/')[-1][:-4]
    # tempOutputFile = tempfile.gettempdir() + '/' + inputFileSuffix + ".txt"
    tempOutputFile = util.make_temp_name()
    cwd = os.getcwd()
    cmd = '/usr/bin/python3  /samplers/KUS.py --samples=' + str(numSolutions) + ' ' + '--outputfile ' + tempOutputFile
    cmd += ' ' + str(os.path.abspath(inputFile)) # + ' > /dev/null 2>&1'
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
        sol = set([int(x) for x in sol.strip().split(" ") if x != ''])
        sol_tmp = ""
        for x in sol:
            sol_tmp += str(x) + " "

        sol = sol_tmp
        solList.append(sol.strip())

    os.unlink(str(tempOutputFile))

    solreturnList = solList
    if len(solList) > numSolutions:
        solreturnList = random.sample(solList, numSolutions)
    return list(map(util.solstr_to_frozenset, solreturnList))

def getSolutionFromKUS2(inputFile, numSolutions, newSeed):

    inputFileSuffix = inputFile.split('/')[-1][:-4]
    # tempOutputFile = tempfile.gettempdir() + '/' + inputFileSuffix + ".txt"
    tempOutputFile = util.make_temp_name()
    cwd = os.getcwd()
    cmd = f'/usr/bin/python3 /samplers/KUS2.py -c {os.path.abspath(inputFile)} -n {numSolutions} -s {newSeed}  | grep -E -v "^c" > {tempOutputFile}'
    # if args.verbose:
    print("cmd: ", cmd)
    # os.chdir(str(os.getcwd()) + '/samplers')
    os.system(cmd)
    # os.chdir(str(cwd))

    with open(tempOutputFile, 'r') as f:
        lines = f.readlines()

    os.unlink(str(tempOutputFile))

    return list(map(util.solstr_to_frozenset, lines))

def getSolutionFromJSampler(inputFile, numSolutions, newSeed):

    inputFileSuffix = inputFile.split('/')[-1][:-4]
    # tempOutputFile = tempfile.gettempdir() + '/' + inputFileSuffix + ".txt"
    tempOutputFile = util.make_temp_name()
    cwd = os.getcwd()
    cmd = f'/julia/bin/julia /jsampler/sampler.jl {os.path.abspath(inputFile)} {numSolutions} {50}  | grep -E -v "^c" > {tempOutputFile}'
    # if args.verbose:
    print("cmd: ", cmd)
    # os.chdir(str(os.getcwd()) + '/samplers')
    os.system(cmd)
    # os.chdir(str(cwd))

    with open(tempOutputFile, 'r') as f:
        lines = f.readlines()

    os.unlink(str(tempOutputFile))

    return list(map(util.solstr_to_frozenset, lines))

def getSolutionFromKSampler(inputFile, numSolutions, newSeed):

    inputFileSuffix = inputFile.split('/')[-1][:-4]
    # tempOutputFile = tempfile.gettempdir() + '/' + inputFileSuffix + ".txt"
    tempOutputFile = util.make_temp_name()
    cwd = os.getcwd()
    cmd = f'/ksampler/sampler.r {os.path.abspath(inputFile)} {numSolutions} {50}  | grep -E -v "^c" > {tempOutputFile}'
    # if args.verbose:
    print("cmd: ", cmd)
    # os.chdir(str(os.getcwd()) + '/samplers')
    os.system(cmd)
    # os.chdir(str(cwd))

    with open(tempOutputFile, 'r') as f:
        lines = f.readlines()

    os.unlink(str(tempOutputFile))

    return list(map(util.solstr_to_frozenset, lines))

def getSolutionFromRSampler(inputFile, numSolutions, newSeed):

    inputFileSuffix = inputFile.split('/')[-1][:-4]
    # tempOutputFile = tempfile.gettempdir() + '/' + inputFileSuffix + ".txt"
    tempOutputFile = util.make_temp_name()
    cwd = os.getcwd()
    cmd = f'/ksampler/rsampler.r {os.path.abspath(inputFile)} {numSolutions} {50000}  | grep -E -v "^c" > {tempOutputFile}'
    # if args.verbose:
    print("cmd: ", cmd)
    # os.chdir(str(os.getcwd()) + '/samplers')
    os.system(cmd)
    # os.chdir(str(cwd))

    with open(tempOutputFile, 'r') as f:
        lines = f.readlines()

    os.unlink(str(tempOutputFile))

    return list(map(util.solstr_to_frozenset, lines))

def getSolutionFromDistAware(inputFile, numSolutions, newSeed):

    inputFileSuffix = inputFile.split('/')[-1][:-4]
    # tempOutputFile = tempfile.gettempdir() + '/' + inputFileSuffix + ".txt"
    tempOutputFile = util.make_temp_name() + ".txt"

    # creating the file to configure the sampler
    # dbsConfigFile = tempfile.gettempdir() + '/' + inputFileSuffix + ".a"
    dbsConfigFile = util.make_temp_name() + ".a"

    logFile = util.make_temp_name() + ".txt"
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

    solreturnList = solList
    if len(solList) > numSolutions:
        solreturnList = random.sample(solList, numSolutions)
    return list(map(util.solstr_to_frozenset, solreturnList))

def getSolutionFromWalkSAT(inputFile, numSolutions, newSeed):

    tempOutputFile = util.make_temp_name()
    cmd = f'/deps/Walksat-master/Walksat_v56/walksat -numsol {numSolutions} -tries {numSolutions * 10} -seed {newSeed} -arc4 -sol {inputFile} > {tempOutputFile}'
    # if args.verbose:
    print("cmd: ", cmd)
    # os.chdir(str(os.getcwd()) + '/samplers')
    os.system(cmd)
    # os.chdir(str(cwd))

    with open(tempOutputFile, 'r') as f:
        lines = f.readlines()

    solList = []

    for line in lines:
        if line.startswith("s0 "):
            sol = line[3:]
            # print(sol.strip().split(" "))
            sol = set([int(x) for x in sol.strip().split(" ") if x != ''])
            sol_tmp = ""
            for x in sol:
                sol_tmp += str(x) + " "

            sol = sol_tmp
            solList.append(sol.strip())

    os.unlink(str(tempOutputFile))

    solreturnList = solList
    if len(solList) > numSolutions:
        solreturnList = random.sample(solList, numSolutions)
    return list(map(util.solstr_to_frozenset, solreturnList))

def getSolutionFromBDDSampler(inputFile, numSolutions, newSeed):
    if not os.path.isfile(f'{inputFile}.bdd'):
        print(f"BDDSampler Error: File '{inputFile}.bdd' does not exist.")
        sys.exit(1)

    # must construct: ./approxmc3 -s 1 -v2 --sampleout /dev/null --samples 500
    inputFileSuffix = inputFile.split('/')[-1][:-4]
    # tempOutputFile = tempfile.gettempdir() + '/' + inputFileSuffix + ".txt"
    tempOutputFile = util.make_temp_name()

    cmd = f'/deps/BDDSampler/bin/BDDSampler -seed {int(newSeed)} {numSolutions} {inputFile}.bdd > {tempOutputFile}'
    print("cmd: ", cmd)
    os.system(cmd)
    # os.system(f"ls \"{tempfile.gettempdir()}\"")

    with open(tempOutputFile, 'r') as f:
        lines = f.readlines()

    solList = []
    for line in lines:
        line = line.strip()
        l = []
        i = 1
        for j in line.split(' '):
            if j == '0':
                l.append(-1 * i)
                i += 1
            elif j == '1':
                l.append(i)
                i += 1
        solList.append(' '.join(map(str, frozenset(l))))

    print(f"BDDSampler: {numSolutions} generated: {len(solList)}, seed: {newSeed}")
    solreturnList = solList
    if len(solList) > numSolutions:
        solreturnList = random.sample(solList, numSolutions)

    os.unlink(str(tempOutputFile))
    return list(map(util.solstr_to_frozenset, solreturnList))

def getSamplerFunction(sampler, cnf_file):
    UNIGEN3 = "unigen3"
    SPUR = "spur"
    STS = "sts"
    STSsingle = "sts1"
    SMARCH = "smarch"
    LOOKAHEAD = "lookahead"
    QUICKSAMPLER = "quicksampler"
    CMSGEN = "cmsgen"
    KUS = "kus"
    KUS2 = "kus2"
    DISTAWARE = "distaware"
    WALKSAT = "walksat"
    JSAMPLER = "jsampler"
    KSAMPLER = "ksampler"
    RSAMPLER = "rsampler"
    BDDSAMPLER = "bddsampler"

    sampler_fn = getSolutionFromUniGen3

    if sampler == UNIGEN3:
        sampler_fn = getSolutionFromUniGen3
    elif sampler == SPUR:
        sampler_fn = getSolutionFromSpur
    elif sampler == STS:
        sampler_fn = getSolutionFromSTS
    elif sampler == STSsingle:
        sampler_fn = getSolutionFromSTSsingle
    elif sampler == SMARCH:
        sampler_fn = getSolutionFromSMARCH
    elif sampler == LOOKAHEAD:
        sampler_fn = getSolutionFromLookahead
    elif sampler == QUICKSAMPLER:
        sampler_fn = getSolutionFromQuickSampler
    elif sampler == CMSGEN:
        sampler_fn = getSolutionFromCMSsampler
    elif sampler == KUS:
        sampler_fn = getSolutionFromKUS
    elif sampler == KUS2:
        sampler_fn = getSolutionFromKUS2
    elif sampler == DISTAWARE:
        sampler_fn = getSolutionFromDistAware
        create_features_dict(cnf_file)
    elif sampler == WALKSAT:
        sampler_fn = getSolutionFromWalkSAT
    elif sampler == JSAMPLER:
        sampler_fn = getSolutionFromJSampler
    elif sampler == KSAMPLER:
        sampler_fn = getSolutionFromKSampler
    elif sampler == RSAMPLER:
        sampler_fn = getSolutionFromRSampler
    elif sampler == BDDSAMPLER:
        sampler_fn = getSolutionFromBDDSampler

    return sampler_fn

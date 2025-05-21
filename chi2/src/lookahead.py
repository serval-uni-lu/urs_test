from subprocess import getoutput
import random
import tempfile
import os
import math
import uuid
import argparse


def is_sat(cnf, assignment, eqvs):
    dimacs = make_temp_name()
    cnf.to_file(dimacs, assignment, eqvs)

    Z3_cmd = "z3 -dimacs '{}' | grep -E \"^s SATISFIABLE$\""
    cmd = Z3_cmd.format(dimacs)
    # print(cmd)
    s = getoutput(cmd)
    # print(s)
    os.remove(dimacs)
    return s == "s SATISFIABLE"

def get_sample(cnf, assignment, eqvs):
    dimacs = make_temp_name()
    cnf.to_file(dimacs, assignment, eqvs)

    Z3_cmd = "z3 -dimacs '{}' | grep -E \"^v \"| sed 's/^v //g'"
    cmd = Z3_cmd.format(dimacs)
    s = getoutput(cmd)

    os.remove(dimacs)
    return s

def bsat(cnf, assignment, eqvs, nb):
    dimacs = make_temp_name()

    Z3_cmd = "z3 -dimacs '{}' | grep -E \"^v \"| sed 's/^v //g'"

    s = []
    for i in range(0, nb):
        cnf.to_file(dimacs, assignment, eqvs, exclude = s)
        cmd = Z3_cmd.format(dimacs)

        out = getoutput(cmd)
        if out.strip() == "":
            break
        s.append([int(x) for x in out.strip().split(' ') if x != ' ' and x != ''])

    os.remove(dimacs)
    return s

def get_cmsgen_samples(cnf, assignment, eqvs, nb):
    dimacs = make_temp_name()
    smp_file = make_temp_name()

    cnf.to_file(dimacs, assignment, eqvs)

    CMS_cmd = "/cmsgen/build/cmsgen --seed \"{}\" --samplefile \"{}\" --samples \"{}\" \"{}\""
    seed = random.randint(a = 0, b = 2**32 - 1)

    cmd = CMS_cmd.format(seed, smp_file, nb, dimacs)
    s = getoutput(cmd)

    samples = set()
    f = open(smp_file)
    for line in f:
        samples.add(frozenset([int(x) for x in line.strip().split(" ") if x != '' and x != '0']))

    os.remove(smp_file)
    os.remove(dimacs)
    return samples

def get_sts_samples(cnf, assignment, eqvs, nb):
    dimacs = make_temp_name()

    cnf.to_file(dimacs, assignment, eqvs)

    STS_cmd = "/deps/STS/core/STS -rnd-seed=\"{}\" -nsamples=1 -k=\"{}\" \"{}\" | grep -E '^s ' | sed 's/^s //g'"
    seed = random.randint(a = 0, b = 2**32 - 1)

    cmd = STS_cmd.format(seed, nb, dimacs)
    s = getoutput(cmd)

    samples = set()
    for line in s.split('\n'):
        samples.add(frozenset([int(x) for x in line.strip().split(" ") if x != '' and x != '0']))

    os.remove(dimacs)
    return samples

# def call_smp_fast(dimacsfile_):
#     SMP_cmd = "/smp_fast \"{}\" > \"{}.smp\""
#     dimacs = getoutput(SMP_cmd.format(dimacsfile_, dimacsfile_))
#     os.rename(dimacsfile_ + ".smp", dimacsfile_)

def make_temp_name(dir = tempfile.gettempdir()):
    return os.path.join(dir, str(uuid.uuid1()))

def sign(x):
    return x / abs(x)

def get_most_balanced_var(samples):
    d = dict()
    for s in samples:
        for l in s:
            d[abs(l)] = 0

    for s in samples:
        for l in s:
            d[abs(l)] += sign(l)

    min_d = len(samples)
    for k in d:
        if abs(d[k]) < min_d:
            min_d = abs(d[k])


    all_vars = []
    for k in d:
        if abs(d[k]) == min_d:
            all_vars.append(k)

    c = random.choice(all_vars)
    imbalance = ((len(samples) / 2) + d[c]) / len(samples)
    return c, imbalance

def get_most_balanced_pair(samples):
    var = dict()
    for s in samples:
        for l in s:
            var[abs(l)] = set()

    for s in samples:
        for l in s:
            var[abs(l)].add(sign(l))

    non_const = [x for x in var if len(var[x]) == 2]

    pairs = dict()
    for i in range(0, len(non_const) - 1):
        for j in range(i + 1, len(non_const)):
            pairs[(non_const[i], non_const[j])] = 0

    for s in samples:
        for p in pairs:
            a, b = p

            if (a in s and b in s) or (-a in s and -b in s) :
                pairs[p] += 1
            elif (-a in s and b in s) or (a in s and -b in s) :
                pairs[p] -= 1

    min_d = len(samples)
    for p in pairs:
        if abs(pairs[p]) < min_d:
            min_d = abs(pairs[p])

    all_pairs = []
    for p in pairs:
        if abs(pairs[p]) == min_d:
            all_pairs.append(p)

    if len(all_pairs) == 0:
        return (0, 0), -1

    c = random.choice(all_pairs)
    imbalance = ((len(samples) / 2) + pairs[c]) / len(samples)
    return c, imbalance


class DIMACS:
    def __init__(self, path):
        self.nb_v = 0
        self.nb_c = 0
        self.cls = []
        with open(path) as f:
            for l in f:
                if l.startswith("p cnf "):
                    sp = l.split(" ")
                    self.nb_v = int(sp[2])
                    self.nb_c = int(sp[3])
                elif not l.startswith("c"):
                    self.cls.append(l)

    def to_file(self, dst, assignment, eqvs = [], exclude = []):
        d = open(dst, 'w')
        d.write(f"p cnf {self.nb_v} {self.nb_c + len(assignment) + len(exclude) + 2 * len(eqvs)}\n")
        for i in self.cls:
            d.write(i.strip())
            d.write("\n")
        for a in assignment:
            d.write(f"{a} 0\n")

        for e in exclude:
            for a in e:
                d.write(f"{-1 * a} ")
            d.write(f"0\n")

        for e in eqvs:
            a, b = e
            d.write(f"{-a} {b} 0\n")
            d.write(f"{a} {-b} 0\n")
        d.close()

def sampler(cnf, k, verbose, use_eqv):
    assignment = []
    eqvs = []

    while len(bsat(cnf, assignment, eqvs, 2)) >= 2:
        # samples = get_cmsgen_samples(cnf, assignment, eqvs, k)
        samples = get_sts_samples(cnf, assignment, eqvs, k)
        v, v_imbalance = get_most_balanced_var(samples)
        if verbose:
            print(f"vi: {v_imbalance}")

        if use_eqv:
            p, p_imbalance = get_most_balanced_pair(samples)
            if abs(0.5 - v_imbalance) <= abs(0.5 - p_imbalance):
                assignment.append(random.choice([-1 * v, v]))
            else:
                a, b = p
                eqvs.append((random.choice([-a, a]), random.choice([-b, b])))
        else:
            if random.random() < v_imbalance:
                assignment.append(v)
            else:
                assignment.append(-v)

    return assignment, eqvs


def main(base, nb, k, v, use_eqv):
    orig = DIMACS(base)

    for i in range(0, nb):
        assignment, eqvs = sampler(orig, k, v, use_eqv)
        print(get_sample(orig, assignment, eqvs))
        # print(assignment)
        # print(eqvs)


parser = argparse.ArgumentParser()
parser.add_argument("-c", "--cnf", type=str)
parser.add_argument("-k", type=int, default=50)
parser.add_argument("-nb", type=int, default=50)
parser.add_argument("-v", type=bool, const=True, nargs='?', default=False)
parser.add_argument("-eqv", type=bool, const=True, nargs='?', default=False)

args = parser.parse_args()

main(args.cnf, args.nb, args.k, args.v, args.eqv)

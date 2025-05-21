from scipy.stats import chi2
from scipy.stats import chisquare
from scipy.stats import poisson
import scipy.special
import numpy as np

import random
import sys
import math
import time

# import pycosat

def count_repeats(samples, sample_size):
    # d = set()

    # nb = 0
    # i = 0
    # for line in samples:
    #     i += 1
    #     if line in d:
    #         nb += 1
    #     d.add(line)
    #     if i >= sample_size:
    #         break
    # return nb
    d = dict()
    i = 0
    for line in samples:
        i += 1
        if line in d:
            d[line] += 1
        else:
            d[line] = 1
        if i >= sample_size:
            break

    res = 0
    for i in d:
        res += scipy.special.comb(d[i], 2, exact = True, repetition = False)

    return res

def monobit(settings):
    total_mc = settings.nnf.get_node(1).mc

    even = 0
    for i in range(0, len(settings.nnf.get_node(1).mc_by_nb_vars)):
        if i % 2 == 0:
            even += settings.nnf.get_node(1).mc_by_nb_vars[i]

    uneven = total_mc - even

    if uneven == 0 or even == 0:
        print("pv 1")
        print("is uniform True")
        print("timeout even/uneven is zero")
        return

    sample_size = max(settings.batch_size, math.ceil(total_mc * settings.min_elem_per_cell / min(uneven, even)))
    print(f"sample size: {math.ceil(sample_size / settings.batch_size) * settings.batch_size}")

    if settings.batch_size == -1:
        settings.batch_size = sample_size

    for _ in range(0, settings.repeats):
        observed = [0, 0]
        nb_samples = 0
        while nb_samples < sample_size:
            samples = settings.sampler_fn(settings.cnf_file, settings.batch_size, random.randint(0, 2**31 - 1))
            nb_samples += len(samples)

            print(str(nb_samples) + " / " + str(sample_size))

            for s in samples:
                n = 0
                for f in s:
                    if f > 0:
                        n += 1

                observed[n % 2] += 1

            if settings.max_end_time <= time.time():
                print("timeout True")
                return

        # building the ovserved = [even, uneven] array
        # if nb features is even then modulo 2 becomes 0 thus the index in the array
        # similarily for an uneven nb features

        expected = [even * nb_samples / total_mc, uneven * nb_samples / total_mc]

        X2, pv = chisquare(observed, expected, ddof = 0)
        crit = chi2.ppf(1 - settings.significance_level, df = len(observed) - 1)

        if pv <= 0:
            pv = sys.float_info.min
        if math.isnan(pv):
            pv = 1

        print(f"X2 {X2}")
        print(f"crit {crit}")
        print(f"pv {pv}")
        # print(f"u {X2 <= crit}")
        print(f"is uniform {pv > settings.significance_level}")
        print("timeout False")
        # print(f"is uniform {pv > settings.significance_level and pv < 1 - settings.significance_level}")
        # print(f"X2: {X2} ({pv})\ncrit: {crit}")
        # print(X2 <= crit)

# Test with n bits mod k instead of k=2
def modbit(settings):
    Q = settings.modbit_q

    total_mc = settings.nnf.get_node(1).mc

    expected = []
    for i in range(0, Q):
        expected.append(0)

    for i in range(0, len(settings.nnf.get_node(1).mc_by_nb_vars)):
        idx = i % Q
        expected[idx] += settings.nnf.get_node(1).mc_by_nb_vars[i]

    m = total_mc
    nb_bins = 0
    for i in expected:
        if i != 0:
            nb_bins += 1
            m = min(m, i)

    sample_size = max(settings.batch_size, math.ceil(total_mc * settings.min_elem_per_cell / m))
    print(f"sample size: {math.ceil(sample_size / settings.batch_size) * settings.batch_size}")

    if settings.batch_size == -1:
        settings.batch_size = sample_size

    for _ in range(0, settings.repeats):
        observed = [0] * len(expected)
        nb_samples = 0

        while nb_samples < sample_size:
            samples = settings.sampler_fn(settings.cnf_file, settings.batch_size, random.randint(0, 2**31 - 1))
            nb_samples += len(samples)

            print(str(nb_samples) + " / " + str(sample_size))

            for s in samples:
                n = 0
                for f in s:
                    if f > 0:
                        n += 1

                observed[n % Q] += 1

            if settings.max_end_time <= time.time():
                print("timeout True")
                return

        # building the ovserved = [even, uneven] array
        # if nb features is even then modulo 2 becomes 0 thus the index in the array
        # similarily for an uneven nb features

        r_observed = []
        r_expected = []
        for i in range(0, len(expected)):
            if expected[i] != 0:
                r_observed.append(observed[i])
                r_expected.append(expected[i] * nb_samples / total_mc)

        X2, pv = chisquare(r_observed, r_expected, ddof = 0)
        crit = chi2.ppf(1 - settings.significance_level, df = len(observed) - 1)

        if pv <= 0:
            pv = sys.float_info.min
        if math.isnan(pv):
            pv = 1

        print(f"X2 {X2}")
        print(f"crit {crit}")
        print(f"pv {pv}")
        # print(f"u {X2 <= crit}")
        print(f"is uniform {pv > settings.significance_level}")
        print("timeout False")
        # print(f"is uniform {pv > settings.significance_level and pv < 1 - settings.significance_level}")
        # print(f"X2: {X2} ({pv})\ncrit: {crit}")
        # print(X2 <= crit)

def frequency_variables(settings):
    total_mc = settings.nnf.get_node(1).mc
    expected = settings.nnf.get_node(1).mc_by_var

    m = settings.nnf.get_node(1).mc
    for i in expected:
        if expected[i] != 0 and expected[i] != total_mc:
            m = min(m, settings.nnf.get_node(1).mc_by_var[i])

    sample_size = max(settings.batch_size, math.ceil(total_mc * settings.min_elem_per_cell / m))
    print(f"sample size: {math.ceil(sample_size / settings.batch_size) * settings.batch_size}")

    if settings.batch_size == -1:
        settings.batch_size = sample_size

    for _ in range(0, settings.repeats):
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
            samples = settings.sampler_fn(settings.cnf_file, settings.batch_size, random.randint(0, 2**31 - 1))
            nb_samples += len(samples)


            for s in samples:
                for f in s:
                    if f > 0 and expected[abs(f)] != 0 and expected[abs(f)] != total_mc:
                        observed[abs(f)] += 1

            print(str(nb_samples) + " / " + str(sample_size))

            if settings.max_end_time <= time.time():
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
                    crit = chi2.ppf(1 - (settings.significance_level / nb_tested_vars), df = len(r_observed) - 1)

                    if pv <= 0:
                        pv = sys.float_info.min
                    if math.isnan(pv):
                        pv = 1

                    print(f"v{i} X2 {X2}")
                    print(f"v{i} crit {crit}")
                    print(f"v{i} pv {pv}")
                    # print(f"u {X2 <= crit}")
                    pvs.append(pv)
                    is_uniform = is_uniform and pv > (settings.significance_level / nb_tested_vars)
                    print(f"v{i} is uniform {pv > (settings.significance_level / nb_tested_vars)}")
                    # print(f"is uniform {pv > settings.significance_level and pv < 1 - settings.significance_level}")
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


def frequency_nb_variables(settings):
    total_mc = settings.nnf.get_node(1).mc
    expected = settings.nnf.get_node(1).mc_by_nb_vars

    m = settings.nnf.get_node(1).mc
    nb_bins = 0
    for i in expected:
        if i != 0:
            nb_bins += 1
            m = min(m, i)

    sample_size = max(settings.batch_size, math.ceil(total_mc * settings.min_elem_per_cell / m))
    print(f"sample size: {math.ceil(sample_size / settings.batch_size) * settings.batch_size}")

    if settings.batch_size == -1:
        settings.batch_size = sample_size


    for _ in range(0, settings.repeats):
        nb_samples = 0
        observed = [0] * len(expected)

        while nb_samples < sample_size:
            samples = settings.sampler_fn(settings.cnf_file, settings.batch_size, random.randint(0, 2**31 - 1))
            nb_samples += len(samples)

            print(str(nb_samples) + " / " + str(sample_size))

            for s in samples:
                n = 0
                for f in s:
                    if f > 0:
                        n += 1
                observed[n] += 1

            if settings.max_end_time <= time.time():
                print("timeout True")
                return


        #for s in samples:
        #    n = 0
        #    for f in s.strip().split(" "):
        #        if int(f) > 0:
        #            n += 1
        #    observed[n] += 1

        r_observed = []
        r_expected = []
        for i in range(0, len(expected)):
            if expected[i] != 0:
                r_observed.append(observed[i])
                r_expected.append(expected[i] * nb_samples / total_mc)

        #print(r_observed)
        #print(r_expected)

        X2, pv = chisquare(r_observed, r_expected, ddof = 0)
        crit = chi2.ppf(1 - settings.significance_level, df = len(r_observed) - 1)

        if pv <= 0:
            pv = sys.float_info.min
        if math.isnan(pv):
            pv = 1

        print(f"X2 {X2}")
        print(f"crit {crit}")
        print(f"pv {pv}")
        # print(f"u {X2 <= crit}")
        print(f"is uniform {pv > settings.significance_level}")
        print("timeout False")
        # print(f"is uniform {pv > settings.significance_level and pv < 1 - settings.significance_level}")
        # print(f"X2: {X2} ({pv})\ncrit: {crit}")
        # print(X2 <= crit)

def birthday_test(settings):
    # implementation of:
    # https://www.pcg-random.org/posts/birthday-test.html

    desired = settings.bday_prob
    factor = 0
    if desired < 1.0:
        factor = math.sqrt(-2.0 * math.log(desired))
    else:
        factor = math.sqrt(2.0 * desired)

    rng_range = settings.nnf.get_node(1).mc
    sample_size = math.ceil(factor * math.sqrt(rng_range))
    #expected = sample_size - rng_range * -1 * math.expm1(sample_size * math.log1p(-1 / rng_range))

    if settings.batch_size == -1:
        settings.batch_size = sample_size

    expected = scipy.special.binom(sample_size, 2) / rng_range
    p_zero = math.exp(-1 * expected)

    print(f"sample size: {math.ceil(sample_size / settings.batch_size) * settings.batch_size}")
    print(f"used sample size: {sample_size}")
    print(f"expected: {expected}")
    print(f"prob of zero duplicates: {p_zero}")

    for _ in range(0, settings.repeats):
        samples = []
        while len(samples) < sample_size:
            samples.extend(settings.sampler_fn(settings.cnf_file, settings.batch_size, random.randint(0, 2**31 - 1)))

            print(str(len(samples)) + " / " + str(sample_size))

            if settings.max_end_time <= time.time():
                print("timeout True")
                return

        repeats = count_repeats(samples, sample_size)

        print(f"expected repeats: {expected}")
        print(f"observed repeats: {repeats}")

        pv = 2 * min(poisson.sf(repeats - 1, expected), poisson.cdf(repeats, expected))

        if pv <= 0:
            pv = sys.float_info.min
        if math.isnan(pv):
            pv = 1

        # print(f"X2 {X2}")
        # print(f"crit {crit}")
        print(f"pv {pv}")
        # print(f"u {X2 <= crit}")
        print(f"is uniform {pv > settings.significance_level}")
        print("timeout False")

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


def pearson_chisquared(settings):
    rng_range = settings.nnf.get_node(1).mc
    expected = [settings.min_elem_per_cell] * rng_range
    sample_size = rng_range * settings.min_elem_per_cell

    if settings.batch_size == -1:
        settings.batch_size = sample_size


    print(f"sample size: {math.ceil(sample_size / settings.batch_size) * settings.batch_size}")

    for _ in range(0, settings.repeats):
        #samples = []
        nb_samples = 0
        bins = dict()
        nb_unsat = 0
        while nb_samples < sample_size:
            samples = settings.sampler_fn(settings.cnf_file, settings.batch_size, random.randint(0, 2**31 - 1))
            for s in samples:
                nb_samples += 1
                if s in bins:
                    bins[s] += 1
                else:
                    bins[s] = 1

                    # print(ts)
                    # tmp_s = [[x] for x in ts]
                    # s = pycosat.solve(tmp_s + dimacs.cls)
                    # print(s)

                    # if s == 'UNSAT':
                    #     print('error UNSAT')
                    #     nb_unsat += 1
                if nb_samples >= sample_size:
                    break
                
            print(str(nb_samples) + " / " + str(sample_size))

            if settings.max_end_time <= time.time():
                print("timeout True")
                return

        #observed = make_bins(samples, sample_size)

        observed = []
        for s in bins:
            observed.append(bins[s])

        while len(observed) < rng_range:
            observed.append(0)
        # print(observed)

        print(f"obs len: {len(observed)}")
        print(f"exp len: {len(expected)}")
        print(f"rng range: {rng_range}")
        print(f"nb unsat: {nb_unsat}")
        X2, pv = chisquare(observed, expected, ddof = 0)
        crit = chi2.ppf(1 - settings.significance_level, df = rng_range - 1)

        if pv <= 0:
            pv = sys.float_info.min
        if math.isnan(pv):
            pv = 1

        print(f"X2 {X2}")
        print(f"crit {crit}")
        print(f"pv {pv}")
        # print(f"u {X2 <= crit}")
        print(f"is uniform {pv > settings.significance_level}")
        print("timeout False")
        # print(f"is uniform {pv > settings.significance_level and pv < 1 - settings.significance_level}")
        # print(f"X2: {X2} ({pv})\ncrit: {crit}")
        # print(X2 <= crit)

def getTestFunction(test_str):
    test_fn = frequency_variables

    if "monobit" == test_str:
        test_fn = monobit
    elif "modbit" == test_str:
        test_fn = modbit
    elif "freq_var" == test_str:
        test_fn = frequency_variables
    elif "sfpc" == test_str:
        test_fn = frequency_nb_variables
    elif "chisquared" == test_str:
        test_fn = pearson_chisquared
    elif "birthday" == test_str:
        test_fn = birthday_test

    return test_fn

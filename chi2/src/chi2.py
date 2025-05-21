#/usr/bin/env python3

import dDNNF
import DIMACS
import argparse
import time

import samplers
import tests
import util

parser = argparse.ArgumentParser()
parser.add_argument("-c", "--cnf", type=str, help="path to the cnf formula")
# parser.add_argument("-k", type=int, default=50)
parser.add_argument("-a", type=float, default=0.01, help="set the significance level")
parser.add_argument("-n", type=int, default=1, help="number of times to repeat the test")
parser.add_argument("-b", "--batch_size", type=int, default=20, help="set the batch size, i.e. the number of solutions asked to the sampler. If it is k<0 then it is converted to abs(k) * #models of formula")
parser.add_argument("-s", "--sampler", type=str, default="unigen3", help="set the sampler to test")
parser.add_argument("--test", type=str, default="freq_var", help="set the statistical test")

parser.add_argument("-t", "--timeout", type=int, default="18000", help="sets the timeout for the test, if exceeded the test is cancelled")

parser.add_argument("--min_elem_per_cell", type=int, default=20, help="set the minimum expected elements per cell for chi-squared tests")
# parser.add_argument("--effect_size", type=float, default=0.3, help="set the individual effect size for the power analysis")
# parser.add_argument("--power", type=float, default=0.99, help="set the target power of individual tests for the power analysis")
parser.add_argument("--bday_prob", type=float, default=10, help="set the desired probability for the birthday test if v < 1.0, otherwise it sets the expected number of repeats")
parser.add_argument("--modbit_q", type=int, default=2, help="set the desired value Q for the modbit test")



args = parser.parse_args()
settings = util.Settings(args)

test_fn = tests.getTestFunction(args.test)
test_fn(settings)

settings.clean_up()

exec_time = time.time() - settings.start_time
print(f"total time: {exec_time}")

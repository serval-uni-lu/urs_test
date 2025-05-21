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

parser.add_argument("-t", "--timeout", type=int, default="18000", help="sets the timeout for the test, if exceeded the test is cancelled")

parser.add_argument("--min_elem_per_cell", type=int, default=20, help="set the minimum expected elements per cell for chi-squared tests")
# parser.add_argument("--effect_size", type=float, default=0.3, help="set the individual effect size for the power analysis")
# parser.add_argument("--power", type=float, default=0.99, help="set the target power of individual tests for the power analysis")
parser.add_argument("--bday_prob", type=float, default=10, help="set the desired probability for the birthday test if v < 1.0, otherwise it sets the expected number of repeats")
parser.add_argument("--modbit_q", type=int, default=2, help="set the desired value Q for the modbit test")

parser.add_argument("--monobit", type=bool, const=True, nargs='?', default=False, help="if set then the monobit test will be executed")
parser.add_argument("--modbit", type=bool, const=True, nargs='?', default=False, help="if set then the modbit test will be executed")
parser.add_argument("--freq_var", type=bool, const=True, nargs='?', default=False, help="if set then the var frequency test will be executed")
parser.add_argument("--freq_nb_var", type=bool, const=True, nargs='?', default=False, help="if set then the number of selected var frequency test will be executed")
parser.add_argument("--bday", type=bool, const=True, nargs='?', default=False, help="if set then the birthday test will be executed")
parser.add_argument("--chisquared", type=bool, const=True, nargs='?', default=False, help="if set then Pearson's chi-squared test will be executed")


args = parser.parse_args()


# significance_level = args.a
# cnf_file = args.cnf
# batch_size = args.batch_size

# min_elem_per_cell = args.min_elem_per_cell
# effect_size = args.effect_size
# power = args.power

# max_time = args.timeout

# args.sampler = args.sampler.lower()
# sampler_fn = samplers.getSamplerFunction(args.sampler)

# start_time = time.time()
# max_end_time = time.time() + max_time

# dimacs = DIMACS.from_file(cnf_file)
# dDNNF_path = util.compute_dDNNF(cnf_file)

# nnf = dDNNF.from_file(dDNNF_path)
# nnf.annotate_mc()

# print(f"model count: {nnf.get_node(1).mc}")
# 
# if batch_size < 0:
#     batch_size = abs(batch_size) * nnf.get_node(1).mc

# print(f"batch size: {batch_size}")

settings = util.Settings(args)

if args.monobit:
    tests.monobit(settings)
if args.modbit:
    tests.modbit(settings)
if args.freq_var:
    tests.frequency_variables(settings)
if args.freq_nb_var:
    tests.frequency_nb_variables(settings)
if args.bday:
    tests.birthday_test(settings)
if args.chisquared:
    tests.pearson_chisquared(settings)

# os.unlink(settings.dDNNF_path)
settings.clean_up()

exec_time = time.time() - settings.start_time
print(f"total time: {exec_time}")

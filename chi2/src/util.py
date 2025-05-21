import os
import uuid
import tempfile
import time
from subprocess import getoutput

import DIMACS
import dDNNF
import samplers

def make_temp_name(dir = tempfile.gettempdir()):
    return os.path.join(dir, str(uuid.uuid1()))

def get_mc(cnf):
    D4_cmd = '/d4/d4 -mc \"{}\" 2>&1 | grep -E \'^s [0-9]+$\' | sed \'s/^s //g\''
    r = getoutput(D4_cmd.format(cnf))
    return int(r)

def compute_dDNNF(cnf):
    tmp = make_temp_name()
    D4_cmd = '/d4/d4 \"{}\" -dDNNF -out=\"{}\" 2>&1'
    r = getoutput(D4_cmd.format(cnf, tmp))
    return tmp

def solstr_to_frozenset(sol):
    t1 = filter(lambda x : x != '', sol.split(' '))
    return frozenset(filter(lambda x : x != 0, map(int, t1)))

class Settings:
    def __init__(self, args):
        self.significance_level = args.a
        self.cnf_file = args.cnf
        self.batch_size = args.batch_size
        self.min_elem_per_cell = args.min_elem_per_cell
        self.max_time = args.timeout

        self.modbit_q = args.modbit_q
        self.repeats = args.n
        self.bday_prob = args.bday_prob

        args.sampler = args.sampler.lower()
        self.sampler_fn = samplers.getSamplerFunction(args.sampler, self.cnf_file)

        self.start_time = time.time()
        self.max_end_time = time.time() + self.max_time

        self.dimacs = DIMACS.from_file(self.cnf_file)
        self.dDNNF_path = compute_dDNNF(self.cnf_file)

        self.nnf = dDNNF.from_file(self.dDNNF_path)
        self.nnf.annotate_mc()


        print(f"model count: {self.nnf.get_node(1).mc}")

        if self.batch_size < 0:
            self.batch_size = abs(self.batch_size) * self.nnf.get_node(1).mc

        print(f"batch size: {self.batch_size}")

    def clean_up(self):
        os.unlink(self.dDNNF_path)

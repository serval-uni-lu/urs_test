import argparse
import dDNNF

parser = argparse.ArgumentParser()
parser.add_argument("-f", "--file", type=str)

args = parser.parse_args()

dDNNF_file = args.file

dDNNF.from_file(dDNNF_file)

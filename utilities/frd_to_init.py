#!/usr/bin/python

import argparse
import logging

parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("--input-file", help="Name of the input .frd file")
parser.add_argument("--output-file", default="T.init", help="Name of the input .frd file")

args = parser.parse_args()

input_file = open(args.input_file)
output_file = open(args.output_file, "w")

logging.info("Loading FRD file: ", input_file)

txt = input_file.read()
last_idx = txt.rfind("CL")
last_temp = txt[last_idx:]

logging.info("Writing output file: ", output_file)

for line in last_temp.split("\n")[3:]:
    cols = line.split()
    if len(cols) == 3:
        output_file.write(cols[1] + "," + cols[2] + "\n")

output_file.close()

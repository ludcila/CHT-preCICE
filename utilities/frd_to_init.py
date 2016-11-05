#!/usr/bin/python

import argparse
import logging

log_level = getattr(logging, "INFO", None)
logging.basicConfig(level=log_level)

parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("--input-file", help="Name of the input .frd file")
parser.add_argument("--output-file", default="T.init", help="Name of the input .frd file")
parser.add_argument("--time", default=-1, help="Time for which to extract the data")

args = parser.parse_args()

input_file = open(args.input_file)
output_file = open(args.output_file, "w")
target_time = float(args.time)

logging.info("Loading FRD file: " + args.input_file)

if target_time >= 0:

    logging.info("Extracting data for time = " + str(target_time) + " from frd file")

    time_found = False

    for line in input_file:

        # Specified time has been found
        if time_found:

            # Temperature data for the specified time
            if line.find(" -1") == 0:
                cols = line.split()
                output_file.write(cols[1] + "," + cols[2] + "\n")

            # End of temperature data for the specified time
            if line.find(" -3") == 0:
                break

        # Specified time not yet found
        else:

            # Process line containing time information
            if line.find("CL") >= 0:
                time = float(line.split()[2])
                if time == target_time:
                    time_found = True

    if time_found:
        logging.info("Output written to: " + args.output_file)
    else:
        logging.error("Specified time not found in frd file")

else:

    logging.info("Extracting last time step from frd file")

    txt = input_file.read()
    last_idx = txt.rfind("CL")
    last_temp = txt[last_idx:]

    for line in last_temp.split("\n")[3:]:
        cols = line.split()
        if len(cols) == 3:
            output_file.write(cols[1] + "," + cols[2] + "\n")

    logging.info("Output written to: " + args.output_file)

output_file.close()

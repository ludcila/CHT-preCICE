#!/usr/bin/python

"""
This utility resets the paths in the .export file of a Code_Aster simulation
 - The path to adapter.comm will be: $ASTER_ADAPTER_ROOT + "/adapter.comm"
   (The environment variable ASTER_ADAPTER_ROOT must be set with the path to the Code_Aster adapter)
 - The path to config.comm will be ../config.comm (in the directory of the coupled case)
 - The absolute path is removed from the rest of the files (e.g. /my/path/to/the/case/mesh.mmed --> mesh.mmed)
"""

import os
import argparse

parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("--input-file", help="Name of the input .export file")
args = parser.parse_args()
filename = args.input_file
export_file = open(filename, "r")
input_text = export_file.read()
export_file.close()
export_file = open(filename, "w")
adapter_root = os.getenv("ASTER_ADAPTER_ROOT")

for line in input_text.split("\n"):
    if len(line) > 0:
        if line[0] == "F":
            parts = line.split()
            if "adapter.comm" in parts[2]:
                parts[2] = adapter_root + "/" + "adapter.comm"
            elif "config.comm" in parts[2]:
                parts[2] = "../config.comm"
            else:
                from_idx = parts[2].rfind("/") + 1
                parts[2] = parts[2][from_idx:]
            export_file.write(" ".join(parts) + "\n")
        else:
            export_file.write(line + "\n")

export_file.close()

import yaml
import argparse
import pprint


def yaml2comm(input):
    input_data = yaml.load(input)
    output = "settings = \\\n"
    output = output + pprint.pformat(input_data)
    return output

parser = argparse.ArgumentParser()
parser.add_argument("--input-file", default="config.yml")
parser.add_argument("--output-file", default="config.comm")
args = parser.parse_args()

input_file = open(args.input_file)
input_string = input_file.read()

output_file = open(args.output_file, "w")
output_string = yaml2comm(input_string)
output_file.write(output_string)
output_file.close()


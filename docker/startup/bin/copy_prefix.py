#! /usr/bin/python3

'''
auth: gabriel solomon
email: gasolomon@purestorage.com
desc: copies files with the same prefix
'''

import argparse
import os

# parse arguments
parser = argparse.ArgumentParser(
    description='copies files with the same prefix.\nfor example,' +
    ' copy_prefix.py foo bar')
parser.add_argument('from_prefix', nargs=1, type=str,
                    help='from file prefix')
parser.add_argument('to_prefix', nargs=1, type=str,
                    help='from file prefix')
parser.add_argument(
    '-c', '--copy',
    action='store_true', help='copy files with the prefix specified by from')
parser.add_argument(
    '-m', '--move',
    action='store_true', help='move files with the prefix specified by from')

args = parser.parse_args()
# print('args=', args)

# arguments passed
from_prefix = args.from_prefix[0]
to_prefix = args.to_prefix[0]


# color printing support
# https://en.wikipedia.org/wiki/ANSI_escape_code#Colors
# color examples
# print(colors.RED + 'test' + colors.END)
# print(colors.WHITE + colors.RED_BG + 'test' + colors.END)
# print(colors.WHITE + 'test' + colors.END)
class colors:
    RED = '\033[91m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    PINK = '\033[95m'
    CYAN = '\033[96m'
    WHITE = '\033[97m'
    DARK_YELLOW = '\033[33m'
    DARK_CYAN = '\033[36m'
    DARK_MAGENTA = '\033[35m'
    RED_BG = '\033[101m'
    GREEN_BG = '\033[102m'
    YELLOW_BG = '\033[103m'
    BLUE_BG = '\033[104m'
    PINK_BG = '\033[105m'
    CYAN_BG = '\033[106m'
    END = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


def main():

    cmd = cmd_long = ''
    if args.copy:
        cmd = 'cp'
        cmd_long = 'copy'
    if args.move:
        cmd = 'mv'
        cmd_long = 'move'

    dir_files = os.listdir()
    # print('dir_files=', dir_files)

    found = False
    for filename in dir_files:
        # find match on prefix
        if from_prefix in filename:
            found = True
            # get suffix of filename
            suffix = filename.split(from_prefix)[1]
            # create new filename
            to_name = to_prefix + suffix
            # copy or move from_name to to_name
            run_cmd = cmd + ' ' + filename + ' ' + to_name
            print(colors.PINK + 'STATUS: ' + cmd_long + ' ' +
                  filename + ' to ' + to_name + colors.END)
            print('\t' + run_cmd)
            os.system(run_cmd)

    if not found:
        print(colors.RED + 'ERROR: no files found with the prefix ' +
              from_prefix + colors.END)


main()

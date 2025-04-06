#! /usr/bin/python3

'''
auth: gabriel solomon
desc: converts all sim results into csv files
'''

import argparse
import logging
import os

# globals
INPUT_DIR = r'/home/gsolomon/avswu-veins/simulations/veins_avswu/results'
OUTPUT_DIR = r'/home/gsolomon/avswu/saved-sim-results'

# parse arguments
parser = argparse.ArgumentParser(
    description='converts all sim results into csv files')
parser.add_argument(
    '-v', '--verbose',
    action='store_true', help='verbose debug output')

args = parser.parse_args()
# print('args=', args)


# color printing support
# https://en.wikipedia.org/wiki/ANSI_escape_code#colors
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


class CustomFormatter(logging.Formatter):
    '''
    custom formatter
    '''
    # format_prefix = f"{colors.PINK}%(asctime)s{colors.END} " \
    #     f"{colors.BLUE}%(name)s{colors.END} " \
    #     f"{colors.CYAN}(%(filename)s:%(lineno)d){colors.END} "
    format_prefix = f"{colors.CYAN}%(asctime)s{colors.END} " \
        f"{colors.CYAN}%(filename)s:%(lineno)d{colors.END} "
    format_suffix = "%(levelname)s - %(message)s"

    FORMATS = {
        logging.DEBUG:
        format_prefix + colors.WHITE + format_suffix + colors.END,
        logging.INFO:
        format_prefix + colors.GREEN + format_suffix + colors.END,
        logging.WARNING:
        format_prefix + colors.YELLOW + format_suffix + colors.END,
        logging.ERROR:
        format_prefix + colors.RED + format_suffix + colors.END,
        logging.CRITICAL:
        format_prefix + colors.RED_BG + format_suffix + colors.END
    }

    # date format
    DATEFMT = '%Y-%m-%d %H:%M:%S'

    def format(self, record):
        log_fmt = self.FORMATS.get(record.levelno)
        formatter = logging.Formatter(log_fmt, datefmt=self.DATEFMT)
        return formatter.format(record)


def create_custom_logger(log_level):
    '''
    setup console handler with a higher log level
    '''

    logger = logging.getLogger(__name__)
    logger.setLevel(log_level)

    # create console handler with a higher log level
    ch = logging.StreamHandler()
    ch.setLevel(log_level)

    ch.setFormatter(CustomFormatter())

    logger.addHandler(ch)

    return logger


def get_input_files(logger):
    # input
    files = os.listdir(INPUT_DIR)
    input_files = [elem for elem in files if '.sca' in elem]

    # output
    output_files = [elem.replace('General', 'sim') for elem in input_files]
    output_files = [elem.replace('=', '-')
                    for elem in output_files]
    output_files = [elem.replace(',', '-')
                    for elem in output_files]
    output_files = [elem.replace('.sca', 'scalar.sca')
                    for elem in output_files]

    sim_files = zip(input_files, output_files)

    # for e in sim_files:
    #     logger.debug(e)

    return sim_files


def convert_to_csv(logger, sims):
    for input, output in sims:
        input_file_path = f'{INPUT_DIR}/{input}'
        output_file_path = f'{OUTPUT_DIR}/{output}'.replace('.sca', '.csv')
        # logger.info(f'converting {input_file_path}')
        cmd = \
            f'scavetool x {input_file_path} -o {output_file_path}'
        logger.debug(f'cmd={cmd}')
        status = os.system(cmd)
        if status == 0:
            logger.info(f'writing {output_file_path}')
        else:
            logger.error(f'unable to write {output_file_path}')
            exit(-1)


def main():

    # gets args
    is_verbose = args.verbose

    # setup color logger
    logger = create_custom_logger(
        logging.DEBUG if is_verbose else logging.INFO)

    logger.debug(f'input_dir={INPUT_DIR}')
    logger.debug(f'output_dir={OUTPUT_DIR}')

    # get sims
    sims = get_input_files(logger)

    # convert sims
    convert_to_csv(logger, sims)


main()

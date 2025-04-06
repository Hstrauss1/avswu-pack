#! /usr/bin/python3

'''
auth: gabriel solomon
desc: setups a simulation for running based on args
'''

import argparse
import logging
import os

# globals
SIM_DIR = r'/home/gsolomon/avswu-veins/simulations/veins_avswu'

# usage: sim-setup.py -s 10 -k 6 -m 3 -d 400 -c 600

# parse arguments
parser = argparse.ArgumentParser(
    description='runs multiple simulations based on parameters')
parser.add_argument(
    '-s', '--size', nargs=1, type=int,
    default=[1], help='size in MiB')
parser.add_argument(
    '-k', '--k', nargs=1, type=int,
    default=[6], help='erasure k')
parser.add_argument(
    '-m', '--m', nargs=1, type=int,
    default=[3], help='erasure m')
parser.add_argument(
    '-d', '--delivery_av', nargs=1, type=int,
    default=[40], help='number of delivery av')
parser.add_argument(
    '-c', '--client_av', nargs=1, type=int,
    default=[60], help='number of client av')
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


def copy_route_xml(logger, n_dav, n_cav):
    src = f'{SIM_DIR}/avswu_trips/avswu_trips-{n_dav}dav-{n_cav}cav.rou.xml'
    dst = f'{SIM_DIR}/erlangen.rou.xml'
    logger.debug(f'src={src}')
    logger.debug(f'dst={dst}')

    # if file exists, copy into place
    cmd = f'cp {src} {dst}'
    status = os.system(cmd)
    if status == 0:
        logger.info(f'updated {dst}')
    else:
        logger.error(
            f'unable to find route file for n_dav={n_dav}, n_cav={n_cav}')
        quit()


# ${size=1}
# sed -i -E "s/size=[0-9]+/temp2/g" omnetpp.ini
def update_key_value(logger, key, value):
    old = f'{key}=[0-9]+'
    new = f'{key}={value}'
    cmd = f'sed -i -E \'s/{old}/{new}/g\' {SIM_DIR}/omnetpp.ini'
    logger.debug(f'cmd={cmd}')

    status = os.system(cmd)
    if status != 0:
        logger.error('unable to successfully run ' +
                     cmd + ' status=' + str(status))
        quit()


def update_omnetpp_ini(logger, size, k, m, n_dav, n_cav):
    update_key_value(logger, 'size', size)
    update_key_value(logger, 'k', k)
    update_key_value(logger, 'm', m)
    update_key_value(logger, 'dav', n_dav)
    update_key_value(logger, 'cav', n_cav)

    logger.info(f'updated {SIM_DIR}/omnetpp.ini')


def main():

    # gets args
    size = args.size[0]
    k = args.k[0]
    m = args.m[0]
    n_dav = args.delivery_av[0]
    n_cav = args.client_av[0]
    is_verbose = args.verbose

    # setup color logger
    logger = create_custom_logger(
        logging.DEBUG if is_verbose else logging.INFO)

    logger.debug(f'args={args}')

    # copy erlangen.rou.xml file in to place
    copy_route_xml(logger, n_dav, n_cav)

    # update omnetpp.ini file with sim values
    update_omnetpp_ini(logger, size, k, m, n_dav, n_cav)


main()

#! /usr/bin/python3

'''
auth: gabriel solomon
desc: produces a rou.xml file suitable for use with avswu
[#dav, #cav, -period (default to 0.5),
maxSpeed (default to 30mph, 48.3kmh)] -> rou.xml by running randomTrips.py
and altering the trips.xml file
'''

import argparse
import logging
import os
import math

import random
import xml.etree.ElementTree as ET

# globals
SEED = 123456

# parse arguments
erlangen_net_file = \
    '/home/gsolomon/avswu-veins/simulations/veins_avswu/erlangen.net.xml'
parser = argparse.ArgumentParser(
    description='produces a rou.xml file suitable '
    + 'for use with avswu simulation')
parser.add_argument('-o', '--output_file', nargs=1, type=str,
                    default=['avswu_trips.rou.xml'], help='output file')
parser.add_argument('-i', '--input_file', nargs=1, type=str,
                    default=[erlangen_net_file], help='input file')
parser.add_argument(
    '-d', '--delivery_av', nargs=1, type=int,
    default=[200], help='number of delivery av')
parser.add_argument(
    '-c', '--client_av', nargs=1, type=int,
    default=[300], help='number of client av')
parser.add_argument(
    '-p', '--period', nargs=1, type=float,
    default=[0.5], help='period of how often avs depart')
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


def main():

    # gets args
    output_file = args.output_file[0]
    input_file = args.input_file[0]
    n_dav = args.delivery_av[0]
    n_cav = args.client_av[0]
    period = float(args.period[0])
    end_time = math.floor((n_dav + n_cav) * period)
    is_verbose = args.verbose

    # setup color logger
    logger = create_custom_logger(
        logging.DEBUG if is_verbose else logging.INFO)

    msg = 'output_file=%s n_dav=%d n_cav=%d period=%f' % \
        (output_file, n_dav, n_cav, period)
    logger.info(msg)

    # course supplies, computers, software, hardware, equipment
    msg = 'creating random routes for %d delivery av, %d client av' % (
        n_dav, n_cav)
    logger.info(msg)

    # run randomTrips.py
    # ~/src/sumo-1.11.0/tools/randomTrips.py -n erlangen.net.xml
    # -e 300 --period 0.5 --route-file trips.xml --validate --seed 123456
    tmp_file = '/tmp/' + \
        os.path.basename(__file__) + '-' + str(os.getpid()) + '-temp.xml'
    logger.debug(tmp_file)
    random_trips_cmd = '/home/gsolomon/src/sumo-1.11.0/tools/randomTrips.py'
    input_cmd = '-n ' + input_file
    end_cmd = '-e ' + str(end_time)
    period_cmd = '--period ' + str(period)
    output_cmd = '--route-file ' + tmp_file
    extra_cmd = '--validate --seed ' + str(SEED)

    cmd = random_trips_cmd + ' ' + input_cmd + ' ' + output_cmd + \
        ' ' + end_cmd + ' ' + period_cmd + ' ' + extra_cmd
    logger.info(cmd)

    status = os.system(cmd)
    if status != 0:
        logger.error('unable to successfully run ' +
                     cmd + ' status=' + str(status))
        quit()

    # seed python random generator (so i'ts repeatable)
    random.seed(SEED)

    # read and parse xml file
    tree = ET.parse(tmp_file)
    root = tree.getroot()

    # add vehicle type
    # change from: <vehicle id="2" depart="1.00">
    # to: <vehicle id="1" type='deliveryType' depart="1.00">
    cav = n_cav
    dav = n_dav
    # vehicle_type_list = ['clientType', 'deliveryType']
    for vehicle in root:
        logger.debug(vehicle.tag)
        logger.debug(vehicle.attrib)
        # select a car type randomly
        # vehicle_type = random.choice(vehicle_type_list)
        # select delivery av first, then remaining are client vehicles
        vehicle_type = 'deliveryType'

        logger.debug('n_cav=%d, n_dav=%d' % (n_cav, n_dav))

        # check if we have vehicle type available
        if vehicle_type == 'clientType':
            if cav == 0:
                vehicle_type = 'deliveryType'
                dav -= 1
            else:
                cav -= 1
        elif vehicle_type == 'deliveryType':
            if dav == 0:
                vehicle_type = 'clientType'
                cav -= 1
            else:
                dav -= 1
        else:
            logger.error('unknown vehicle type')

        logger.debug('selected vehicle_type=%s' % vehicle_type)

        vehicle.attrib['type'] = vehicle_type

    logger.info('created %d delivery vehicles and %d client vehicles' %
                (n_dav, n_cav))

    # add vehicle types to file
    '''
    <vType id='clientType' accel="2.6" decel="4.5" sigma="0.5"
    length="2.5" minGap="2.5" maxSpeed="48.3" color="0,0,1"/>
    <vType id='deliveryType' accel="2.6" decel="4.5" sigma="0.5"
    length="2.5" minGap="2.5" maxSpeed="48.3" color="0,1,0"/>
    '''
    for av_type in ['clientType', 'deliveryType']:
        av = ET.Element("vType")
        av.attrib['id'] = av_type
        av.attrib['accel'] = '2.6'
        av.attrib['decel'] = '4.5'
        av.attrib['sigma'] = '0.5'
        av.attrib['length'] = '2.5'
        av.attrib['minGap'] = '2.5'
        # kilometers per hour
        av.attrib['maxSpeed'] = '48.3'
        # set vehicle color
        if av_type == 'clientType':
            av.attrib['color'] = '0,0,1'
        else:
            av.attrib['color'] = '0,1,0'

        root.append(av)

    # write tmp file
    tmp_file2 = '/tmp/' + \
        os.path.basename(__file__) + '-' + str(os.getpid()) + '-temp2.xml'
    ET.indent(tree, space="\t", level=0)
    tree.write(tmp_file2)

    # re-order the vehicle types to top of xml file

    # read the tmp2 file line by line
    tmp_file2_handle = open(tmp_file2, 'r')
    line_list = tmp_file2_handle.readlines()

    # reorder the lines
    n = len(line_list)
    dav_line = line_list[n-2]
    cav_line = line_list[n-3]
    new_line_list = []
    # shift values down items
    for i in range(n-1):
        # add <route>
        if i == 0:
            new_line_list.append(line_list[i])
        # add cav and dav types
        if i == 1:
            new_line_list.append(cav_line)
        if i == 2:
            new_line_list.append(dav_line)
        # add remaining departing vehicles
        if i >= 3:
            new_line_list.append(line_list[i-2])
    # add </route>
    new_line_list.append(line_list[n-1])

    logger.info('saving '+output_file)
    output = open(output_file, 'w')
    output.writelines(new_line_list)
    output.close()

    # remove tmp files
    os.system('rm -f ' + tmp_file)
    os.system('rm -f ' + tmp_file2)


main()

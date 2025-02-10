import numpy as np
import logging
import re
import pandas as pd


# create and configure logger

# configure log output format
# FORMAT = \
#     '[%(asctime)s:%(filename)s:%(funcName)s:%(lineno)s:' + \
#     '%(levelname)-8s] %(message)s'
FORMAT = '[%(levelname)s] %(message)s'
logging.basicConfig(format=FORMAT)

# get logger, for my code only
logger = logging.getLogger('sim-graph')

# set level
# logger.setLevel(logging.INfO)
logger.setLevel(logging.DEBUG)

# disable matplotlib findfont debugging messages
# logging.getLogger('matplotlib.font_manager').disabled = True

# dirs and files
BASE_DIR = r'/Users/jerom/Documents/GitHub/avswu'
RESULTS_DIR = BASE_DIR + r'/saved-sim-results'
PLOTS_DIR = BASE_DIR + r'/sim-plots'


def read_data_frame(csv_file):
    csv_file_path = RESULTS_DIR + '/' + csv_file
    df = pd.read_csv(csv_file_path)
    return df


def create_sw_update_time_df(csv_df):
    '''
    creates a sw_update_time_df
    '''
    # filter by row by sw_time to update
    col = 'name'
    row_val = 'sw_time_to_update_from_client_shard_request_time'
    df = csv_df.loc[csv_df[col] == row_val]

    # select module and mean columns
    df = df[['module', 'mean']]

    # rename columns [module,mean] -> [node,time (s)]
    df.rename(columns={'module': 'node_id', 'mean': 'time'}, inplace=True)

    # extract node number, from AvswuScenario.node[93].appl
    for index, row in df.iterrows():
        s = row['node_id']
        # match number with capture group
        m = re.match(r'.*\[(.+)\]', s)
        # if m exists, replace with value
        if m:
            node_val = int(m.group(1))
            df.loc[index, 'node_id'] = node_val
        else:
            logger.error(f'unable to find node_id value in s={s}')

    # sort by node
    df = df.sort_values(by=['node_id'])

    # create a column for node number
    df['node_number'] = np.arange(1, len(df)+1)
    # rearrange columns
    df = df[['node_number', 'node_id', 'time']]
    print(f'df={df}')

    return df

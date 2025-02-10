import numpy
import os
import pandas as pd
import matplotlib.pyplot as plt
import sw_update_tools as swu_tools
from sw_update_tools import logger as logger

# prefix for plot files
PLOT_FILE_PREFIX = 'number-av'

# prefix for csv files
CSV_FILE_PREFIX = 'sim-size'
# CSV_FILE_PREFIX = 'sim-toy1-size'

# tuples of csv files from sim
# DAV_CAV_ARR = [(40, 60)]
DAV_CAV_ARR = [
    (40, 60),
    (80, 120),
    (120, 180),
    (160, 240),
    (200, 300),
    (240, 360),
    (280, 420),
    (320, 480),
    (360, 540),
    (400, 600)
]

# DAV_CAV_ARR = [(400, 600)]

SW_UPDATE_SIZE_MB = 25
ERASURE_K = 12
ERASURE_M = 6


def save_one_plot(df, dav, cav, plot_file):
    # plot grades and student names
    bar_colors = ['tab:blue']
    ax = df.plot(kind='bar', x='node_number', y='time', figsize=(
        8, 4), legend=False, color=bar_colors)

    # title
    title = f'{dav+cav} AVs ({dav} DAV,{cav} CAV)'
    ax.set_title(title)

    # set labels
    ax.set_xlabel('CAV')
    ax.set_ylabel('Time (s)')

    # plot the graph, and resize to fit in window
    plt.subplots_adjust(bottom=.3, left=.15)

    # make y-axis 0 to 1200s
    plt.ylim(0, 1200)

    plot_file_path = swu_tools.PLOTS_DIR + '/' + plot_file

    # remove the old plot file if it exists
    if os.path.exists(plot_file_path):
        logger.info(f'removing old plot file {plot_file_path}')
        os.remove(plot_file_path)

    # write plot to disk
    fig = ax.get_figure()
    logger.info(f'writing  {plot_file_path}')
    fig.savefig(plot_file_path, dpi=300)
    plt.close(fig)

    # show the plot
    # plt.show()


def save_all_plot(df, plot_file):
    bar_colors = ['tab:blue']
    ax = df.plot(kind='bar', x='total_av', y='mean_time', figsize=(
        8, 4), legend=False, color=bar_colors)

    # title
    title = 'Software Update Time'
    ax.set_title(title)

    # set labels
    ax.set_xlabel('Number of AVs')
    ax.set_ylabel('Mean Time (s)')

    # plot the graph, and resize to fit in window
    plt.subplots_adjust(bottom=.3, left=.15)

    plot_file_path = swu_tools.PLOTS_DIR + '/' + plot_file

    # remove the old plot file if it exists
    if os.path.exists(plot_file_path):
        logger.info(f'removing old plot file {plot_file_path}')
        os.remove(plot_file_path)

    # save plot to disk
    fig = ax.get_figure()
    logger.info(f'writing  {plot_file_path}')
    fig.savefig(plot_file_path, dpi=300)
    # plt.close(fig)

    # save csv to disk
    csv_plot_file_path = plot_file_path.replace('png', 'csv')
    df.to_csv(csv_plot_file_path)

    plt.show()


def save_all_plot_std_dev(df, plot_file):
    line_colors = ['blue', 'black']
    ax = df.plot(x='total_av', y='mean_time',
                 figsize=(8, 4), legend=False, color=line_colors)

    # title
    title = 'Software Update Time'
    ax.set_title(title)

    # set labels
    ax.set_xlabel('Number of AVs')
    ax.set_ylabel('Mean Time (s)')

    # plot the graph, and resize to fit in window
    plt.subplots_adjust(bottom=.3, left=.15)

    # plot for standard deviation
    yerr = df['std_dev']
    x = df['total_av']
    y = df['mean_time']
    ax.errorbar(x, y, yerr, fmt='o', linewidth=2,
                capsize=6,  color='blue', ecolor='black')

    plot_file_path = swu_tools.PLOTS_DIR + '/' + plot_file

    # remove the old plot file if it exists
    if os.path.exists(plot_file_path):
        logger.info(f'removing old plot file {plot_file_path}')
        os.remove(plot_file_path)

    # save plot to disk
    fig = ax.get_figure()
    logger.info(f'writing  {plot_file_path}')
    fig.savefig(plot_file_path, dpi=300)
    # plt.close(fig)

    # save csv to disk
    csv_plot_file_path = plot_file_path.replace('png', 'csv')
    df.to_csv(csv_plot_file_path)

    plt.show()


def main():

    sw_update_time_df = pd.DataFrame(
        columns=['total_av', 'dav', 'cav', 'mean_time', 'std_dev'])

    # dav cav
    n = len(DAV_CAV_ARR)
    for i in range(n):
        dav, cav = DAV_CAV_ARR[i]

        # read data
        file_prefix = f'{CSV_FILE_PREFIX}-{SW_UPDATE_SIZE_MB}-k-{
            ERASURE_K}-m-{ERASURE_M}-dav-{dav}-cav-{cav}'
        csv_file = f'{file_prefix}-scalar.csv'

        csv_df = swu_tools.read_data_frame(csv_file)
        # logger.debug(f'df={csv_df}')

        # create data frame
        df = swu_tools.create_sw_update_time_df(csv_df)
        # logger.debug(f'df={df}')

        # plot data frame
        padded_file_prefix = \
            f'{CSV_FILE_PREFIX}-{str(SW_UPDATE_SIZE_MB).zfill(3)}' + \
            f'-k-{ERASURE_K}-m-{ERASURE_M}-dav-{dav}-cav-{cav}'
        plot_file = f'{
            PLOT_FILE_PREFIX}-{padded_file_prefix}-sw-update-time.png'
        save_one_plot(df, dav, cav, plot_file)

        # calculate mean and standard deviation
        total_av = dav + cav
        time_mean = df.mean()['time']
        time_std = df.std()['time']

        # add to our final plot [number of avs,time_mean,time_std]
        sw_update_time_df.loc[i] = [
            total_av, dav, cav, time_mean, time_std]

    # convert total_av, dav, cav to integers
    sw_update_time_df = sw_update_time_df.astype(
        {'total_av': numpy.uint, 'dav': numpy.uint, 'cav': numpy.uint})

    # print the data
    logger.debug('sw_update_time_df')
    print(sw_update_time_df)

    # plot the results
    file_prefix = f'{CSV_FILE_PREFIX}-{SW_UPDATE_SIZE_MB}-k-{ERASURE_K}-m-{
        ERASURE_M}-dav-all-cav-all'
    all_plot_file = PLOT_FILE_PREFIX + '-' + file_prefix + \
        '-sw-update-time.png'
    save_all_plot(sw_update_time_df, all_plot_file)

    # plot the results w/ std dev
    all_plot_file = PLOT_FILE_PREFIX + '-' + file_prefix + \
        '-all-sw-update-time-std-dev.png'
    save_all_plot_std_dev(sw_update_time_df, all_plot_file)


main()

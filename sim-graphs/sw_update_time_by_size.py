import pandas as pd
import matplotlib.pyplot as plt
import sw_update_tools as swu_tools
from sw_update_tools import logger as logger
import os


# prefix for csv files
# CSV_FILE_PREFIX = 'sim-size'
CSV_FILE_PREFIX = 'sim-toy1-size'

# prefix for plot files
PLOT_FILE_PREFIX = r'size'

# SW_UPDATE_SIZE_MB = [5, 10, 15, 20, 25, 30, 35, 40, 45, 50]
SW_UPDATE_SIZE_MB = [25]

ERASURE_K = 12
ERASURE_M = 6
# N_DAV = 400
# N_CAV = 600
N_DAV = 40
N_CAV = 60


def save_one_plot(df, size, plot_file):
    n = len(df)
    plot_file_path = swu_tools.PLOTS_DIR + '/' + plot_file
    if n == 0:
        logger.warning('no AVs received a software update')
        logger.info(f'removing  {plot_file_path}')
        if os.path.exists(plot_file_path):
            os.remove(plot_file_path)
        return

    # set plot colors and type
    bar_colors = ['tab:blue']
    ax = df.plot(kind='bar', x='node_number', y='time', figsize=(
        8, 4), legend=False, color=bar_colors)

    # title
    title = f'{size} MiB'
    ax.set_title(title)

    # set labels
    ax.set_xlabel('CAV')
    ax.set_ylabel('Time (s)')

    # plot the graph, and resize to fit in window
    plt.subplots_adjust(bottom=.3, left=.15)

    # make y-axis 0 to 1200s
    plt.ylim(0, 1200)

    # show the plot
    # plt.show()

    # remove the old plot file if it exists
    if os.path.exists(plot_file_path):
        logger.info(f'removing old plot file {plot_file_path}')
        os.remove(plot_file_path)

    # write plot to disk
    fig = ax.get_figure()
    logger.info(f'writing {plot_file_path}')
    fig.savefig(plot_file_path, dpi=300)
    plt.close(fig)


def save_all_plot(df, plot_file):
    bar_colors = ['tab:blue']
    ax = df.plot(kind='bar', x='size', y='mean_time', figsize=(
        8, 4), legend=False, color=bar_colors)

    # title
    title = 'Software Update Time'
    ax.set_title(title)

    # set labels
    ax.set_xlabel('Size (MiB)')
    ax.set_ylabel('Mean Time (s)')

    # plot the graph, and resize to fit in window
    plt.subplots_adjust(bottom=.3, left=.15)

    plt.show()

    plot_file_path = swu_tools.PLOTS_DIR + '/' + plot_file

    # remove the old plot file if it exists
    if os.path.exists(plot_file_path):
        logger.info(f'removing old plot file {plot_file_path}')
        os.remove(plot_file_path)

    # save plot to disk
    fig = ax.get_figure()
    logger.info(f'writing {plot_file_path}')
    fig.savefig(plot_file_path, dpi=300)
    # plt.close(fig)

    # save csv to disk
    csv_plot_file_path = plot_file_path.replace('png', 'csv')
    df.to_csv(csv_plot_file_path)


def save_all_plot_std_dev(df, plot_file):
    line_colors = ['blue', 'black']
    ax = df.plot(x='size', y='mean_time',
                 figsize=(8, 4), legend=False, color=line_colors)

    # title
    title = 'Software Update Time'
    ax.set_title(title)

    # set labels
    ax.set_xlabel('Size (MiB)')
    ax.set_ylabel('Mean Time (s)')

    # plot the graph, and resize to fit in window
    plt.subplots_adjust(bottom=.3, left=.15)

    # plot for standard deviation
    yerr = df['std_dev']
    x = df['size']
    y = df['mean_time']
    ax.errorbar(x, y, yerr, fmt='o', linewidth=2,
                capsize=6,  color='blue', ecolor='black')

    plt.show()

    plot_file_path = swu_tools.PLOTS_DIR + '/' + plot_file

    # remove the old plot file if it exists
    if os.path.exists(plot_file_path):
        logger.info(f'removing old plot file {plot_file_path}')
        os.remove(plot_file_path)

    # save plot to disk
    fig = ax.get_figure()
    logger.info(f'writing {plot_file_path}')
    fig.savefig(plot_file_path, dpi=300)
    # plt.close(fig)

    # save csv to disk
    csv_plot_file_path = plot_file_path.replace('png', 'csv')
    df.to_csv(csv_plot_file_path)


def save_delivered_plot(df, plot_file):
    bar_colors = ['tab:blue']
    ax = df.plot(kind='bar', x='size', y='number_delivered', figsize=(
        8, 4), legend=False, color=bar_colors)

    # title
    title = 'Software Updates Delivered'
    ax.set_title(title)

    # set labels
    ax.set_xlabel('Size (MiB)')
    ax.set_ylabel('Number Delivered')

    # plot the graph, and resize to fit in window
    plt.subplots_adjust(bottom=.3, left=.15)

    plt.show()

    plot_file_path = swu_tools.PLOTS_DIR + '/' + plot_file

    # remove the old plot file if it exists
    if os.path.exists(plot_file_path):
        logger.info(f'removing old plot file {plot_file_path}')
        os.remove(plot_file_path)

    # save plot to disk
    fig = ax.get_figure()
    logger.info(f'writing {plot_file_path}')
    fig.savefig(plot_file_path, dpi=300)
    # plt.close(fig)

    # save csv to disk
    csv_plot_file_path = plot_file_path.replace('png', 'csv')
    df.to_csv(csv_plot_file_path)


def main():

    sw_update_time_df = pd.DataFrame(
        columns=['size', 'mean_time', 'std_dev'])

    sw_update_delivered_df = pd.DataFrame(
        columns=['size', 'number_delivered'])

    # dav cav
    n = len(SW_UPDATE_SIZE_MB)
    for i in range(n):
        size = SW_UPDATE_SIZE_MB[i]

        # read data
        file_prefix = f'{CSV_FILE_PREFIX}-{size}-k-{ERASURE_K}-m-{
            ERASURE_M}-dav-{N_DAV}-cav-{N_CAV}'
        csv_file = f'{file_prefix}-scalar.csv'
        csv_df = swu_tools.read_data_frame(csv_file)
        # logger.debug(f'df={csv_df}')

        # create data frame
        df = swu_tools.create_sw_update_time_df(csv_df)
        # logger.debug('df')
        # print(df)

        # plot data frame
        padded_file_prefix = f'{CSV_FILE_PREFIX}-{str(size).zfill(3)}' + \
            f'-k-{ERASURE_K}-m-{ERASURE_M}-dav-{N_DAV}-cav-{N_CAV}'
        plot_file = f'{
            PLOT_FILE_PREFIX}-{padded_file_prefix}-sw-update-time.png'
        save_one_plot(df, size, plot_file)

        # calculate mean and standard deviation
        time_mean = df.mean()['time']
        time_std = df.std()['time']

        # add to our final plot [size,time_mean,time_std]
        sw_update_time_df.loc[i] = [size, time_mean, time_std]

        # count the number of software updates delivered
        number_delivered = len(df)
        sw_update_delivered_df.loc[i] = [size, number_delivered]

    # print the data
    logger.debug('sw_update_time_df')
    print(sw_update_time_df)

    # plot the results
    file_prefix = f'{CSV_FILE_PREFIX}-all-k-{ERASURE_K}-m-{
        ERASURE_M}-dav-{N_DAV}-cav-{N_CAV}'
    all_plot_file = PLOT_FILE_PREFIX + '-' + file_prefix + \
        '-sw-update-time.png'
    save_all_plot(sw_update_time_df, all_plot_file)

    # plot the results w/ std dev
    all_plot_file = PLOT_FILE_PREFIX + '-' + file_prefix + \
        '-all-sw-update-time-std-dev.png'
    save_all_plot_std_dev(sw_update_time_df, all_plot_file)

    # plot the number of sw updates delivered (y-axis) by size of
    # software update (x-axis)
    file_prefix = f'{CSV_FILE_PREFIX}-all-k-{ERASURE_K}-m-{
        ERASURE_M}-dav-{N_DAV}-cav-{N_CAV}'
    all_plot_file = PLOT_FILE_PREFIX + '-' + file_prefix + \
        '-sw-update-delivered.png'
    save_delivered_plot(sw_update_delivered_df, all_plot_file)


main()

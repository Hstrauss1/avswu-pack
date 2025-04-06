#!/bin/bash

# sim controls
# array of (dav,cav) values, number AV dav,cav
dav_cav_pair_arr=("40,60" "80,120" "120,180" "160,240" "200,300" "240,360" "280,420" "320,480" "360,540" "400,600")

size=25
k=12
m=6

dav_cav_pair_arr_length=${#dav_cav_pair_arr[@]}

# funcs to get k and m values given an index
get_dav() {
    local i=$1
    dav_cav_pair="${dav_cav_pair_arr[$i]}\n"
    # split to get k and m values
    dav_cav_arr=(${dav_cav_pair//,/ })
    k=(${dav_cav_arr[0]})
    echo $k
}

get_cav() {
    local i=$1
    dav_cav_pair="${dav_cav_pair_arr[$i]}\n"
    # split to get k and m values
    dav_cav_arr=(${dav_cav_pair//,/ })
    m=(${dav_cav_arr[1]})
    # trim m newlines
    m=$(printf $m | xargs)
    echo $m
}

# log files
LOG_DIR="/home/gsolomon/avswu-veins/simulations/veins_avswu/sim-logs"

# sim running code
for (( i=0 ; i<$dav_cav_pair_arr_length ; i++ ))
do
    # printf "i=$i\n"
    dav=$(get_dav $i)
    cav=$(get_cav $i)
    # printf "k=$k, m=$m\n"

    printf "=================================================================================\n"
    printf "STATUS: running simulation size=$size k=$k m=$m dav=$dav cav=$cav\n"
    printf "=================================================================================\n"
    
    #
    # update the omnetpp.ini file
    #

    # sim-setup.py -s 10 -k 6 -m 3 -d 400 -c 600
    cmd="sim-setup.py -s ${size} -k ${k} -m ${m} -d ${dav} -c ${cav}"
    printf "STATUS: executing cmd=$cmd\n"
    result=$($cmd)
    status=$?
    if [ $status = 0 ]
    then
        printf "STATUS: successfully ran ${cmd}\n"
    fi

    #
    # run the simulation
    #
    mkdir -p $LOG_DIR
    log_file="sim-log-size-${size}-k-${k}-m-${m}-dav-${dav}-cav-${cav}.log"
    log_file_path="$LOG_DIR/$log_file"
    cmd="_simquick"
    printf "STATUS: executing $cmd, writing output to $log_file_path\n"
    result=$($cmd >& $log_file_path)

    #
    # check for simulation success or error
    #

    # check connection
    printf "STATUS: checking simulation if connection refused\n"
    result=$(grep "Connection refused" ${log_file_path})
    connection_refused_status=$?
    if [ $connection_refused_status = 0 ]
    then
        printf "\n"
        printf "=================================================================================\n"
        printf "ERROR: SIMULATION FAILED. simulation size=$size k=$k m=$m dav=$dav cav=$cav\n"
        printf "=================================================================================\n"
        printf "\n"
        printf "STATUS: connection was refused. executing tail -n 3 $log_file\n"
        printf "\n"
        printf "\n"
        grep "Connection refused" $log_file_path | tail -n 3
        printf "\n"
        printf "=================================================================================\n"
        exit -1
    fi

    # Simulation time limit reached
    printf "STATUS: checking simulation simulation time limit reached\n"
    result=$(grep "Simulation time limit reached" ${log_file_path})
    time_limit_status=$?
    if [ ! $time_limit_status = 0 ]
    then
        printf "\n"
        printf "=================================================================================\n"
        printf "ERROR: SIMULATION FAILED. simulation size=$size k=$k m=$m dav=$dav cav=$cav\n"
        printf "=================================================================================\n"
        printf "\n"
        printf "STATUS: simulation time limit was not reached. executing tail $log_file\n"
        printf "\n"
        printf "\n"
        tail $log_file_path
        printf "\n"
        printf "=================================================================================\n"
        exit -1
    fi

    printf "\n"
    printf "=================================================================================\n"
    printf "STATUS: SIMULATION SUCCESS. simulation size=$size k=$k m=$m dav=$dav cav=$cav\n"
    printf "=================================================================================\n"
    printf "\n"
    printf "STATUS: executing tail $log_file\n"
    printf "\n"
    printf "\n"
    tail $log_file_path
    printf "\n"
    printf "=================================================================================\n"

    printf "\n\n\n"

done

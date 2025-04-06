#!/bin/bash

# sim controls
# array of (k,m) values
# k_m_pair_arr=("1,0" "2,1" "6,3" "12,6" "18,9" "24,12" "30,15" "36,18" "42,21" "48,24" "54,27" "60,30" "66,33" "72,36")
k_m_pair_arr=("66,33")

size=25
dav=400
cav=600

k_m_pair_arr_length=${#k_m_pair_arr[@]}

# funcs to get k and m values given an index
get_k() {
    local i=$1
    k_m_pair="${k_m_pair_arr[$i]}\n"
    # split to get k and m values
    k_m_arr=(${k_m_pair//,/ })
    k=(${k_m_arr[0]})
    echo $k
}

get_m() {
    local i=$1
    k_m_pair="${k_m_pair_arr[$i]}\n"
    # split to get k and m values
    k_m_arr=(${k_m_pair//,/ })
    m=(${k_m_arr[1]})
    # trim m newlines
    m=$(printf $m | xargs)
    echo $m
}

# log files
LOG_DIR="/home/gsolomon/avswu-veins/simulations/veins_avswu/sim-logs"

# sim running code
for (( i=0 ; i<$k_m_pair_arr_length ; i++ ))
do
    # printf "i=$i\n"
    k=$(get_k $i)
    m=$(get_m $i)
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

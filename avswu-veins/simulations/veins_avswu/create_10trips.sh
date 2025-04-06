#!/bin/bash

n=1000
step=100
# 2 dav, for every 3 cav
dav_ratio=2
cav_ratio=3

for (( i=0 ; i<$n ; i+=$step ))
do
    av=$(( $i + $step ))
    dav=$(echo "$dav_ratio*($av/($dav_ratio+$cav_ratio))" | bc -l)
    dav=$(echo "($dav)/1" | bc)
    cav=$(echo "$cav_ratio*($av/($dav_ratio+$cav_ratio))" | bc -l)
    cav=$(echo "($cav)/1" | bc)

    printf "=====================================\n"
    printf "i=$i\n"
    printf "av=$av\n"
    printf "dav=$dav\n"
    printf "cav=$cav\n"

    cmd="create_avswu_trip.py -d ${dav} -c ${cav} -o avswu_trips-${dav}dav-${cav}cav.rou.xml"
    printf "executing cmd=$cmd\n"
    $cmd

    printf "=====================================\n"

done


# create_avswu_trip.py -d 400 -c 600 -o avswu_trips-400dav-600cav.rou.xml
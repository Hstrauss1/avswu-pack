#!/bin/bash

# copies, cleans, and re-makes veins-client libs
function _main {

    n=50
    for (( i=0; i<$n ; i++ ))
    do
    printf "\n===============================\nRUNNING SIMULATION $i\n=====================\n"
    simquick $i
    done

    printf "STATUS: done\n"

}

# execute the function
if [ $# != 0 ]
then
    printf "usage: $0n"
    exit
fi
_main $1

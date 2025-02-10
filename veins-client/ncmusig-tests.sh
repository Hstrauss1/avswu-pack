#!/bin/bash

printf "STATUS: running ncmusig tests\n"

n_mult=10
for ((mult=0 ; mult<$n_mult ; mult++ ))
do
    n=100
    step=1
    for (( i=0 ; i<$n ; i++ ))
    do
        signers=$(( ($i+1) * $step ))
        # printf "signers: $signers\n"
        bin/test_non_con_musig -n $signers | grep -E "*,.*"
    done
done


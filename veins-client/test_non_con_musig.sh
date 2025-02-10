#!/bin/bash

if [ $# != 1 ]
then
    echo "usage: run #"
    exit -1
fi

n=$1
for (( i=0; i<${n}; i++ ))
do
    num=$(($i + 1))
    num_signers=$((10+${i}*10))
    printf "${num}/${n} : with ${num_signers} signers"
    test_non_con_musig -n ${num_signers} |  grep "\[info\] signature is"
done
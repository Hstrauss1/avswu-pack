#!/bin/bash

# copies, cleans, and re-makes veins-client libs
function _main {

    local src_branch=$1

    local src_dir="/home/veins/avswu/veins-client"

    # git fetch updated repo copy of avswu
    printf "STATUS: updating avswu repo from ${src_branch} branch of github.com ${src_dir}\n"
    pushd ${src_dir}
    git checkout ${src_branch}
    git status
    git fetch
    git pull
    git status
    popd

    # clean
    printf "STATUS: cleaning CMakeCache.txt and bin...\n"
    rm ${src_dir}/CMakeCache.txt
    rm ${src_dir}/bin/*

    printf "STATUS: compiling...\n"
    pushd ${src_dir}
    cmake .
    make
    popd

    printf "STATUS: done\n"

}

# execute the function
if [ $# != 1 ]
then
    printf "usage: $0 branch_name\n"
    exit
fi
_main $1

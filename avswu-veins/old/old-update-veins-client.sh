#!/bin/bash

# copies, cleans, and re-makes veins-client libs
function _main {

    local src_dir="/home/veins/avswu/veins-client"
    local dst_dir="/home/veins/avswu-veins/veins-client"

    # git fetch in my copy of avswu
    printf "STATUS: updating avswu repo from *** develop *** branch of github.com ${src_dir}\n"
    pushd ${src_dir}
    git checkout develop
    git status
    git fetch
    popd

    # remove old
    printf "STATUS: removing ${dst_dir}\n"
    rm -fr ${dst_dir}
    # copy new
    printf "STATUS: removing, then copy by tar'ing directories excluding ./bin, ./output, and ./.cache ${src_dir}\n"
    rm -fr ${dst_dir}
    mkdir -p ${dst_dir}
    pushd ${src_dir}
    tar cf - --exclude=./bin --exclude=./output --exclude=./.cache --exclude=./CMakeFiles . | (cd ${dst_dir} && tar xvf - )
    # create empty bin and output dirs
    mkdir -p "${dst_dir}/bin"
    mkdir -p "${dst_dir}/output"
    popd

    # clean
    printf "STATUS: cleaning CMakeCache.txt and bin...\n"
    rm ${dst_dir}/CMakeCache.txt
    rm ${dst_dir}/bin/*

    printf "STATUS: make cleaning...\n"
    pushd ${dst_dir}
    make clean
    popd

    printf "STATUS: compiling...\n"
    pushd ${dst_dir}
    cmake .
    make
    popd

    printf "STATUS: done\n"

}

# execute the function
_main

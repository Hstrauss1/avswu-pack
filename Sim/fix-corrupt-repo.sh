#!/bin/bash

# copies, cleans, and re-makes veins-client libs
function _main {

    local repo_dir="/home/veins"
    local repo_name="avswu-veins"

    local corrupt_git_repo="${repo_dir}/${repo_name}"
    local backup="${repo_dir}/BACKUP-${repo_name}"
    local new_clone="${repo_dir}/NEW-CLONE-${repo_name}"

    printf "STATUS: 1. create a backup of the corrupt directory\n"
    local cmd="cp -r ${corrupt_git_repo} ${backup}"
    printf "\tcmd=${cmd}\n"
    ${cmd}

    printf "STATUS:2. make a new clone of the remote repository to a new directory\n"
    printf "          Enter login and personal access token for git repo ${corrup_git_repo}\n"
    local cmd="git clone https://github.com/jeromesolomon/avswu-veins ${new_clone}"
    printf "\tcmd=${cmd}\n"
    ${cmd}

    printf "STATUS:3. delete the corrupt .git subdirectory\n"
    local cmd="rm -fr ${corrupt_git_repo}/.git"
    printf "\tcmd=${cmd}\n"
    ${cmd}

    printf "STATUS:4. move the newly cloned .git subdirectory into the corrupt git repo\n"
    local cmd="mv ${new_clone}/.git ${corrupt_git_repo}"
    printf "\tcmd=${cmd}\n"
    ${cmd}

    printf "STATUS:5. delete the rest of the temporary new clone\n"
    local cmd="rm -fr ${new_clone}"
    printf "\tcmd=${cmd}\n"
    ${cmd}

    printf "\n\n"
    printf "STATUS: backup=${backup}\n"
    printf "STATUS: fixed repo=${corrupt_git_repo}\n"

    printf "STATUS: done\n"

}

# execute the function
_main

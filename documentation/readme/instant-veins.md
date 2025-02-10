# developer workflow for virtualbox

1. start linux laptop
2. start virtual box instant-veins
3. remote host vs code into linux laptop
4. remote host connect into veins

## to make core avswu library changes

5. edit code to make changes on vs code linux laptop, in ~/avswu repo
6. commit changes to repo from vs code
7. run update-avswu.sh (instant-veins) to pull changes to instant veins ~~/avswu repo, and re-build

## to make instant-veins sim changes (~~/avswu-veins repo)

8. edit code to make changes on vs code linux laptop, in ~/avswu repo
9. commit changes to avswu-veins repo
10. run simmake alias
    > simmake
11. start polkadot bc
    > ~/avswu~/avswu-node/start-polka.sh
12. start veins-server
    > cd ~/avswu/veins-server; npm start
13. run simquick alias

    > simquick

14. run simlog for more debugging

    > simlog

15. to save sim results and convert to csv
    > simsaveresults

# instant-veins

## instant veins install

1. download and install instant veins for oracle vm
   https://veins.car2x.org/download/

2. created shared folder to github repo

a. OracleVM->Devices->Shared Folders->Shared Folder Settings
b. create your veins-test folder if its not there
c. from terminal create the vboxshare mount point

to mount the directory

> mkdir /home/veins/vboxshare
> sudo mount -t vboxsf -o uid=1000,gid=1000 sharename /home/veins/vboxshare

## ssh from windows into veins VirtuaBox (ssh to port 3022 on loopback 127.0.0.1)

> ssh -p 3022 veins@127.0.0.1
> login is "veins", password is: "veins"

## instant-veins access to VirtualBox via vs code

## vs code ssh into veins virtualbox vm

https://dev.to/developertharun/easy-way-to-ssh-into-virtualbox-machine-any-os-just-x-steps-5d9i
from VirtualBox

1. instant-veins right-click settings
2. select Network->Adapter 2-> Advanced -> Port forwarding
3. create a port forward from host port 3022 to guest port 22
4. > ssh -p 3022 veins@127.0.0.1

to verify ip address:
(from debian)
ip addr show

File->Open Directory to the root avswu-veins repo directory /home/veins~/avswu-veins

## to edit veins code from vs code

1. start vs code
2. ctrl-shift-P and choose ssh into remote
3. in vscode, open the folder /home/veins~/avswu-veins (which is another separate repo in github)

## how to remote from vscode into virtualbox

ctrl-shift-P, use "remote-ssh" into vmbox from vs code

## install copy/paste

> sudo apt-get -y install autocutsel
> /usr/bin/autocutsel -s CLIPBOARD -fork

# creating a new veins project

> https://www.youtube.com/watch?v=mGvhbrw05sQ

video, message compiler still needs to be set to 6???

compiler flags can go into "makemake" under avswu (rgt click) Properties->Omnet++->Makemake->Options->Custom

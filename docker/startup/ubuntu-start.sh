#!/bin/bash

echo "------------------------------------------------------------------"
echo "STATUS: Starting..."
currentDate=`date`
echo "Date = " $currentDate
echo "------------------------------------------------------------------"

echo "------------------------------------------------------------------"
echo "STATUS:       Starting vncserver..."
echo "NOTE  :       vncserver will ask you for a password.  Enter "
echo "              any password that you wish to have for vnc access"
echo "              into the container."
echo "------------------------------------------------------------------"
# vncserver will complain unless ~/.Xauthority exists
touch ~/.Xauthority

declare -A root_home=/root

# copy the xstartup file that runs xfce4 desktop for tightvnc
mkdir -p $root_home/.vnc
touch $root_home/.Xresources

# start vncserver, 1280×720 lowres hd
export USER="root"
rm -fr /root/.vnc
rm -fr /tmp/.X*
tightvncserver -geometry 1920x1080

# enable copy/paste to/from clipboard via vncserver
/usr/bin/autocutsel -s CLIPBOARD -fork

# run the ssh server
echo "------------------------------------------------------------------"
echo "STATUS:       Starting ssh service..."
echo "------------------------------------------------------------------"
service ssh start

# add bash_profile
echo "------------------------------------------------------------------"
echo "STATUS:       Setting up .bash_profile..."
echo "------------------------------------------------------------------"
chmod a-wx,u+rw,go+r ${root_home}/startup/_.bash_profile
cp ${root_home}/startup/_.bash_profile /etc/skel/.bash_profile
cp ${root_home}/startup/_.bash_profile ${root_home}/.bash_profile

echo "------------------------------------------------------------------"
echo "STATUS:       Copy ~/bin"
echo "------------------------------------------------------------------"
# if files are in bin, we already did the setup
if [ ! -d ${root_home}"/bin" ]
then
    mkdir -p ${root_home}"/bin"
    # copy remaining executables to user template skeleton
    mkdir -p /etc/skel/bin
    cp ${root_home}/startup/bin/* /etc/skel
    # copy to root bin
    mkdir -p ${root_home}/bin
    cp ${root_home}/startup/bin/* ${root_home}/bin

    # cp specific executables to /usr/local/bin, if they are needed for multiple users
    cp ${root_home}/startup/bin/copy_prefix.py /usr/local/bin
    rm ${root_home}"/bin/copy_prefix.py"
fi

# creating a base user, gsolomon w/ password welcome (for now)
echo "------------------------------------------------------------------"
echo "STATUS:       Create gsolomon account..."
echo "------------------------------------------------------------------"
printf "welcome\nwelcome\n" | adduser  --gecos "" gsolomon

# add user to sudoers file
echo "------------------------------------------------------------------"
echo "STATUS:       Adding gsolomon to sudoers file..."
echo "------------------------------------------------------------------"
usermod -aG sudo gsolomon

echo "------------------------------------------------------------------"
echo "STATUS: Done"
echo "------------------------------------------------------------------"

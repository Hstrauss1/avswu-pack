#!/bin/bash

# misc commands to configure and install apps
sudo apt update

# rust
curl --proto '=https' --tlsv1.2 https://sh.rustup.rs -sSf | sh

# python3, pip
sudo apt-get -y install python3
sudo apt-get -y install python3-pip

# compile db
pip install compiledb

# screen recorder
sudo apt-get -y install recordmydesktop

# switch to xserver, so screen recorders work
sudo apt-get update
sudo apt-get install xserver-xorg
# vi /etc/gdm3/custom.conf
# turn off wayland
# reboot to start xserver


# setup recording to work, remove old, reinstall new obs-studio
sudo apt remove obs-studio
sudo snap install obs-studio


# install standard sumo for sumo-gui and netedit
sudo apt-get -y install sumo sumo-tools sumo-doc

# ffmpeg
sudo apt-get -y install ffmpeg

# vlc player
sudo sudo snap install vlc

# openssl
sudo apt-get install libssl-dev

# openssl latest stable version
echo "------------------------------------------------------------------"
echo "STATUS:       Install OpenSSL"
echo "------------------------------------------------------------------"
export OPEN_SSL_URL="https://www.openssl.org/source/openssl-3.3.0.tar.gz"
export OPEN_SSL_VERSION="openssl-3.3.0"
curl -L $OPEN_SSL_URL -o ${OPEN_SSL_VERSION}".tar.gz"
gzip -d ${OPEN_SSL_VERSION}".tar.gz"
tar -xvf ${OPEN_SSL_VERSION}".tar"
rm -f ${OPEN_SSL_VERSION}".tar"
sudo mv ${OPEN_SSL_VERSION} /usr/local
sudo mkdir -p /usr/local/${OPEN_SSL_VERSION}
pushd /usr/local/${OPEN_SSL_VERSION}
./config
make
sudo make install
popd

# display version
openssl version


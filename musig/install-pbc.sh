#!/bin/bash

declare -A user_home=/home/${USER}

declare -A startup=/home/${USER}/vboxshare/docker/startup

echo "------------------------------------------------------------------"
echo "STATUS: Starting..."
currentDate=`date`
echo "Date = " $currentDate
echo "------------------------------------------------------------------"

echo "------------------------------------------------------------------"
echo "STATUS:       Install pbc"
echo "------------------------------------------------------------------"
# PBC, stanford's pbc, pair based cryptography, https://crypto.stanford.edu/pbc/howto.html
sudo apt-get -y install flex
sudo apt-get -y install bison
export PBC_URL="https://crypto.stanford.edu/pbc/files/pbc-0.5.14.tar.gz"
export PBC_VERSION="pbc-0.5.14"
sudo curl -L $PBC_URL -o ${PBC_VERSION}".tar.gz"
sudo gzip -d ${PBC_VERSION}".tar.gz"
sudo tar -xvf ${PBC_VERSION}".tar"
sudo rm -f ${PBC_VERSION}".tar"
sudo mv ${PBC_VERSION} /usr/local
sudo mkdir -p /usr/local/${PBC_VERSION}
pushd /usr/local/${PBC_VERSION}
sudo ./configure
sudo make
sudo make install
popd


echo "------------------------------------------------------------------"
echo "STATUS: Done"
echo "------------------------------------------------------------------"

#!/bin/bash

declare -A user_home=/home/${USER}

declare -A startup=/home/${USER}/vboxshare/docker/startup

echo "------------------------------------------------------------------"
echo "STATUS: Starting..."
currentDate=`date`
echo "Date = " $currentDate
echo "------------------------------------------------------------------"

echo "------------------------------------------------------------------"
echo "STATUS:       Install gmp"
echo "------------------------------------------------------------------"
# GNU Multiple Precision Arithmetic Library, https://gmplib.org
sudo apt-get -y install m4
sudo apt-get -y install -y lzip
sudo apt-get -y install xz-utils
export GMP_URL="https://gmplib.org/download/gmp/gmp-6.3.0.tar.xz"
export GMP_VERSION="gmp-6.3.0"
sudo curl -L $GMP_URL -o ${GMP_VERSION}".tar.xz"
sudo xz -d -v ${GMP_VERSION}".tar.xz"
sudo tar -xvf ${GMP_VERSION}".tar"
sudo rm -f ${GMP_VERSION}".tar"
sudo mv ${GMP_VERSION} /usr/local
sudo mkdir -p /usr/local/${GMP_VERSION}
pushd /usr/local/${GMP_VERSION}
sudo ./configure
sudo make
sudo make install
popd


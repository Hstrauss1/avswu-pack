#!/bin/bash

declare -A user_home=/home/${USER}

declare -A startup=/home/${USER}/vboxshare/docker/startup

echo "------------------------------------------------------------------"
echo "STATUS: Starting..."
currentDate=`date`
echo "Date = " $currentDate
echo "------------------------------------------------------------------"

echo "------------------------------------------------------------------"
echo "STATUS:       Setup mount to repo..."
echo "------------------------------------------------------------------"
sudo mkdir -p ${user_home}/vboxshare
sudo mount -t vboxsf -o uid=1000,gid=1000 avswu ${user_home}/vboxshare

# enable copy/paste to/from clipboard via vncserver
echo "------------------------------------------------------------------"
echo "STATUS:       Adding ctrl-shift copy/paste to/from windows..."
echo "------------------------------------------------------------------"
sudo apt-get -y install autocutsel
/usr/bin/autocutsel -s CLIPBOARD -fork

# add bash_profile
echo "------------------------------------------------------------------"
echo "STATUS:       Setting up .bash_profile..."
echo "------------------------------------------------------------------"
chmod a-wx,u+rw,go+r ${startup}/_.bash_profile
cp ${startup}/_.bash_profile ${user_home}/.bash_profile

echo "------------------------------------------------------------------"
echo "STATUS:       Copy ~/bin"
echo "------------------------------------------------------------------"
# if files in bin
if [ -f ${startup}/bin/* ]
then
    # copy bin files
    mkdir -p ${user_home}/bin
    cp ${startup}/bin/* ${user_home}/bin
    chmod a+x ${user_home}/bin/*
else
    echo "No executables in ${startup}/bin to copy"
fi

echo "------------------------------------------------------------------"
echo "STATUS:       Install cmake for compile_commands.json creation"
echo "------------------------------------------------------------------"
# cmake (for compile_commands.json generation)
sudo apt-get -y install cmake
# clang-format for vscode formatting
sudo apt-get -y install clang-format

echo "------------------------------------------------------------------"
echo "STATUS:       Install grpc"
echo "------------------------------------------------------------------"
# grpc, based on https://grpc.io/docs/languages/cpp/quickstart/#install-grpc
# grpc prereqs
sudo apt-get -y install build-essential autoconf libtool pkg-config
# build gRPC and protocol buffers
export GRPC_INSTALL_DIR=/usr/local
cd ${GRPC_INSTALL_DIR}
sudo git clone --recurse-submodules -b v1.58.0 --depth 1 --shallow-submodules https://github.com/grpc/grpc
export GRPC_DIR=${GRPC_INSTALL_DIR}/grpc
sudo mkdir -p ${GRPC_DIR}/cmake/build
cd ${GRPC_DIR}/cmake/build
sudo cmake -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF -DCMAKE_INSTALL_PREFIX=${GRPC_INSTALL_DIR} ${GRPC_DIR}
sudo make -j 4
sudo make install
# clean up the build directory (to save disk space)
rm -fr ${GRPC_DIR}/cmake/build
cd ~

# install curl
sudo apt-get -y install curl

# GNU Multiple Precision Arithmetic Library, https://gmplib.org
echo "------------------------------------------------------------------"
echo "STATUS:       Install gmp"
echo "------------------------------------------------------------------"
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

# PBC, stanford's pbc, pair based cryptography, https://crypto.stanford.edu/pbc/howto.html
echo "------------------------------------------------------------------"
echo "STATUS:       Install pbc"
echo "------------------------------------------------------------------"
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

# Install nholmann c++ json libs
echo "------------------------------------------------------------------"
echo "STATUS:       Install cpp-json libs"
echo "------------------------------------------------------------------"
export CPP_JSON="json-develop"
export CPP_JSON_URL="https://github.com/nlohmann/json/archive/refs/heads/develop.zip"
sudo curl -L $CPP_JSON_URL -o ${CPP_JSON}".zip"; \
sudo unzip ${CPP_JSON}".zip";
sudo rm -f ${CPP_JSON}".zip";
sudo mv ${CPP_JSON} /usr/local
cd /usr/local/${CPP_JSON}
sudo cmake .
sudo make
sudo make install

# CPABE libraries, http://acsc.cs.utexas.edu/cpabe
echo "------------------------------------------------------------------"
echo "STATUS:       Install CPABE libraries"
echo "------------------------------------------------------------------"
sudo apt-get -y install libglib2.0-dev libgtk2.0-dev

# CPABE libbswabe
# cpabe shared library run-time load path for .so files
echo "------------------------------------------------------------------"
echo "STATUS:       Install CPABE libbswabe"
echo "------------------------------------------------------------------"
export LD_LIBRARY_PATH="/usr/local/lib"
export CPABE_URL="http://acsc.cs.utexas.edu/cpabe/libbswabe-0.9.tar.gz"
export CPABE_VERSION="libbswabe-0.9"
sudo curl -L ${CPABE_URL} -o ${CPABE_VERSION}".tar.gz"
sudo gzip -d ${CPABE_VERSION}".tar.gz"
sudo tar -xvf ${CPABE_VERSION}".tar"
sudo rm -f ${CPABE_VERSION}".tar"
sudo mv ${CPABE_VERSION} /usr/local
cd /usr/local/${CPABE_VERSION}
sudo ./configure CFLAGS="-O3 -Wall -fPIC"
sudo make
sudo make install

# spdlog c++ logging tool
echo "------------------------------------------------------------------"
echo "STATUS:       Install spdlog c++ logging tool"
echo "------------------------------------------------------------------"
cd /usr/local
sudo git clone https://github.com/gabime/spdlog.git
cd /usr/local/spdlog/build
sudo cmake ..
sudo make -j
sudo make install

# GF-Complete, required for Jerasure
echo "------------------------------------------------------------------"
echo "STATUS:       Install GF-Complete"
echo "------------------------------------------------------------------"
sudo git clone https://github.com/ceph/gf-complete
sudo mv gf-complete /usr/local
cd /usr/local/gf-complete
sudo ./autogen.sh
sudo ./configure
sudo make
sudo make install


# Jerasure
echo "------------------------------------------------------------------"
echo "STATUS:       Install Jerasure"
echo "------------------------------------------------------------------"
sudo git clone https://github.com/tsuraan/Jerasure
sudo mv Jerasure /usr/local
cd /usr/local/Jerasure
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/usr/local/lib:/usr/local/gf-complete/src/.libs"
sudo autoreconf --force --install
sudo ./configure
sudo make
sudo make install

# GoogleTest framework
echo "------------------------------------------------------------------"
echo "STATUS:       Install GoogleTest"
echo "------------------------------------------------------------------"
export GOOGLE_TEST_VERSION="1.14.0"
export GOOGLE_TEST="googletest-"${GOOGLE_TEST_VERSION}""
export GOOGLE_TEST_URL="https://github.com/google/googletest/archive/refs/tags/v"${GOOGLE_TEST_VERSION}".tar.gz"
sudo curl -L $GOOGLE_TEST_URL -o ${GOOGLE_TEST}".tar.gz"
sudo gzip -d ${GOOGLE_TEST}".tar.gz"
sudo tar -xvf ${GOOGLE_TEST}".tar"
sudo rm -f ${GOOGLE_TEST}".tar"
sudo mv ${GOOGLE_TEST} /usr/local
cd /usr/local/${GOOGLE_TEST}
sudo cmake .
sudo make
sudo make install

# install clang++ 17
echo "------------------------------------------------------------------"
echo "STATUS:       Install clang++-17"
echo "------------------------------------------------------------------"
cd /usr/local
wget -O - https://apt.llvm.org/llvm.sh
sudo chmod u+x ./llvm.sh
sudo ./llvm.sh 17
sudo rm ./llvm.sh

echo "------------------------------------------------------------------"
echo "STATUS: Done"
echo "------------------------------------------------------------------"

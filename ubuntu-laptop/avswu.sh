!/bin/sh

# same/similar to Dockerfile

# syntax=docker/dockerfile:1
#
# docker file for creating hyperledger fabric ubuntu container
#
# FROM polkadot-grpc-dev

# # environment variables
# declare USER="root"
# declare HOME="/root"

# # container startup scripts
# declare STARTUP="startup"
# export STARTUP_DIR=$HOME/$STARTUP

# # copy bash startup script
# ADD $STARTUP $STARTUP_DIR

# # expose ports for ssh
# EXPOSE 22
# # expose ports for tightvncserver
# # tightvncserver runs display 1 on port 5901 and display 2 on port 5902, et cetera
# EXPOSE 5901

# # create a volume to share data between host and other containers
# VOLUME /shared-volume

#
# specific for this image
#

# GNU Multiple Precision Arithmetic Library, https://gmplib.org
sudo apt-get -y install m4 lzip xz-utils
declare GMP_URL="https://gmplib.org/download/gmp/gmp-6.3.0.tar.xz"
declare GMP_VERSION="gmp-6.3.0"
sudo curl -L $GMP_URL -o ${GMP_VERSION}".tar.xz"; \
    xz -d -v ${GMP_VERSION}".tar.xz"; \
    tar -xvf ${GMP_VERSION}".tar"; \
    rm -f ${GMP_VERSION}".tar"; \
    mv ${GMP_VERSION} /usr/local
sudo mkdir -p /usr/local/${GMP_VERSION}
cd /usr/local/${GMP_VERSION}
sudo ./configure
sudo make; make install

# PBC, stanford's pbc, pair based cryptography, https://crypto.stanford.edu/pbc/howto.html
# PBC is required for CPABE
sudo apt-get -y install flex bison
declare PBC_URL="https://crypto.stanford.edu/pbc/files/pbc-0.5.14.tar.gz"
declare PBC_VERSION="pbc-0.5.14"
sudo curl -L $PBC_URL -o ${PBC_VERSION}".tar.gz"; \
    gzip -d ${PBC_VERSION}".tar.gz"; \
    tar -xvf ${PBC_VERSION}".tar"; \
    rm -f ${PBC_VERSION}".tar"; \
    mv ${PBC_VERSION} /usr/local
sudo mkdir -p /usr/local/${PBC_VERSION}
cd /usr/local/${PBC_VERSION}
sudo ./configure
sudo make; make install

# CPABE libraries, http://acsc.cs.utexas.edu/cpabe
sudo apt-get -y install libglib2.0-dev libgtk2.0-dev

# CPABE libbswabe
# cpabe shared library run-time load path for .so files
export LD_LIBRARY_PATH="/usr/local/lib"
declare CPABE_URL="http://acsc.cs.utexas.edu/cpabe/libbswabe-0.9.tar.gz"
declare CPABE_VERSION="libbswabe-0.9"
sudo curl -L ${CPABE_URL} -o ${CPABE_VERSION}".tar.gz"; \
    gzip -d ${CPABE_VERSION}".tar.gz"; \
    tar -xvf ${CPABE_VERSION}".tar"; \
    rm -f ${CPABE_VERSION}".tar"; \
    mv ${CPABE_VERSION} /usr/local
sudo mkdir -p /usr/local/${CPABE_VERSION}
cd /usr/local/${CPABE_VERSION}
sudo ./configure CFLAGS="-O3 -Wall -fPIC"
sudo make; make install

# cpp-json libraries
declare CPP_JSON="json-develop"
declare CPP_JSON_URL="https://github.com/nlohmann/json/archive/refs/heads/develop.zip"
sudo curl -L $CPP_JSON_URL -o ${CPP_JSON}".zip"; \
    unzip ${CPP_JSON}".zip"; \
    rm -f ${CPP_JSON}".zip"; \
    mv ${CPP_JSON} /usr/local
sudo mkdir -p /usr/local/${CPP_JSON}
cd /usr/local/${CPP_JSON}
sudo cmake .
sudo make; make install

# spdlog c++ logging tool
sudo mkdir -p /usr/local
cd /usr/local
sudo git clone https://github.com/gabime/spdlog.git
sudo mkdir -p /usr/local/spdlog/build
cd /usr/local/spdlog/build
sudo cmake .. && make -j; make install

# GF-Complete, required Jerasure
sudo git clone https://github.com/ceph/gf-complete; \
    mv gf-complete /usr/local
sudo mkdir -p /usr/local/gf-complete
cd /usr/local/gf-complete
sudo ./autogen.sh ; ./configure; make; make install


# Jerasure
sudo git clone https://github.com/tsuraan/Jerasure; \
    mv Jerasure /usr/local
sudo mkdir -p /usr/local/Jerasure
cd /usr/local/Jerasure
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/usr/local/lib:/usr/local/gf-complete/src/.libs"
sudo autoreconf --force --install; ./configure; make; make install

# GoogleTest framework
declare GOOGLE_TEST_VERSION="1.14.0"
declare GOOGLE_TEST="googletest-"${GOOGLE_TEST_VERSION}""
declare GOOGLE_TEST_URL="https://github.com/google/googletest/archive/refs/tags/v"${GOOGLE_TEST_VERSION}".tar.gz"
sudo curl -L $GOOGLE_TEST_URL -o ${GOOGLE_TEST}".tar.gz"; \
    gzip -d ${GOOGLE_TEST}".tar.gz"; \
    tar -xvf ${GOOGLE_TEST}".tar"; \
    rm -f ${GOOGLE_TEST}".tar"; \
    mv ${GOOGLE_TEST} /usr/local
sudo mkdir -p /usr/local/${GOOGLE_TEST}
cd /usr/local/${GOOGLE_TEST}
sudo cmake .
sudo make; make install

# install clang++ 17
sudo mkdir -p /usr/local
cd /usr/local
sudo wget https://apt.llvm.org/llvm.sh; \ 
    chmod u+x ./llvm.sh; \
    ./llvm.sh 17; \
    rm ./llvm.sh

# change to root home dir
cd

# upon creation of the container run the startup script, and do any clean 
# up on the container, set time zone, then start bash
# CMD \
#     # set timezone
#     ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone ; \
#     # run the startup script
#     chmod +x $STARTUP_DIR/ubuntu-start.sh ; \
#     $STARTUP_DIR/ubuntu-start.sh ; \
#     # start a bash shell
#     /bin/bash

!/bin/sh

# same/similar to Dockerfile

# syntax=docker/dockerfile:1
#
# FROM polkadot-dev

# # environment variables
# declare  USER="root"

# # container startup scripts
# declare  STARTUP="startup"
# export STARTUP_DIR=/root/$STARTUP

# # copy bash startup script
# ADD $STARTUP $STARTUP_DIR

# # expose ports for ssh
# EXPOSE 22
# # expose ports for vncserver
# # vncserver runs display 1 on port 5901 and display 2 on port 5902, et cetera
# EXPOSE 5901

# # create a volume to share data between host and other containers
# VOLUME /shared-volume

#
# specific for this image
#

# grpc, based on https://grpc.io/docs/languages/cpp/quickstart/#install-grpc
# grpc prereqs
sudo apt-get -y install build-essential autoconf libtool pkg-config
# build gRPC and protocol buffers
declare  GRPC_INSTALL_DIR=/usr/local
sudo mkdir -p ${GRPC_INSTALL_DIR}
cd ${GRPC_INSTALL_DIR}
sudo git clone --recurse-submodules -b v1.58.0 --depth 1 --shallow-submodules https://github.com/grpc/grpc
declare  GRPC_DIR=${GRPC_INSTALL_DIR}/grpc
sudo mkdir -p ${GRPC_DIR}/cmake/build
cd ${GRPC_DIR}/cmake/build
sudo cmake -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF -DCMAKE_INSTALL_PREFIX=${GRPC_INSTALL_DIR} ${GRPC_DIR}
sudo make -j 4
sudo make install
# clean up the build directory (for diskspace)
sudo rm -fr ${GRPC_DIR}/cmake/build

# upon creation of the container run the startup script, and do any clean 
# up on the container, set time zone, copy bash_profile,
# then start bash

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

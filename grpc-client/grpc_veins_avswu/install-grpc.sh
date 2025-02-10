#!/bin/sh

# grpc, based on https://grpc.io/docs/languages/cpp/quickstart/#install-grpc
# grpc prereqs
apt-get -y install build-essential autoconf libtool pkg-config
# build gRPC and protocol buffers
export GRPC_INSTALL_DIR=/usr/local
mkdir -p ${GRPC_INSTALL_DIR}
cd ${GRPC_INSTALL_DIR}
git clone --recurse-submodules -b v1.58.0 --depth 1 --shallow-submodules https://github.com/grpc/grpc
export GRPC_DIR=${GRPC_INSTALL_DIR}/grpc
mkdir -p ${GRPC_DIR}/cmake/build
cd ${GRPC_DIR}/cmake/build
cmake -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF -DCMAKE_INSTALL_PREFIX=${GRPC_INSTALL_DIR} ${GRPC_DIR}
make -j 4
make install
# clean up the build directory (for diskspace)
rm -fr ${GRPC_DIR}/cmake/build

!/bin/sh

# same/similar to Dockerfile

# syntax=docker/dockerfile:1
#
# FROM rust-dev

# # environment variables
# declare USER="root"

# # container startup scripts
# declare STARTUP="startup"
# ENV STARTUP_DIR=/root/$STARTUP

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

# rust's cargo bin
declare CARGO_BIN="/root/.cargo/bin"

# polkadot prerequisites
sudo apt-get install -y clang && \
    apt-get -y install libssl-dev && \
    apt-get -y install llvm && \
    apt-get -y install libudev-dev && \
    apt-get -y install protobuf-compiler

# polkadot and substrate config
sudo apt-get update

# download the latest version of binaryen webabsembly compiler
# set variables (arg) and environment variable (env) for binaryen webasm compiler
declare BINARYEN_URL="https://github.com/WebAssembly/binaryen/releases/download/version_116/binaryen-version_116-x86_64-linux.tar.gz"
declare BINARYEN_TEMP_FILE="binaryen-temp.tar.gz"
declare BINARYEN_DIR="binaryen-version_116"
ENV PATH=$PATH:/usr/local/bin/binaryen-version_116/bin/

# download latest binaryen version
sudo curl -L $BINARYEN_URL -o ${BINARYEN_TEMP_FILE}".tar.gz"

# uncompress, and extract from tar file
sudo gunzip ${BINARYEN_TEMP_FILE}".tar.gz"; \
    sudo tar -xvf ${BINARYEN_TEMP_FILE}".tar"; \
    sudo rm -fr ${BINARYEN_TEMP_FILE}".tar"

# move the file to /usr/local
sudo mv ${BINARYEN_DIR} /usr/local/bin

# substrate
sudo mkdir -p /usr/local/substrate
cd /usr/local/substrate

# prepare a substrate node
sudo git clone https://github.com/substrate-developer-hub/substrate-node-template

# compile the substrate node template
cd substrate-node-template ; git checkout latest ; ${CARGO_BIN}/cargo build --release ; cd ..

# substrate front-end (ReactJS)
sudo git clone https://github.com/substrate-developer-hub/substrate-front-end-template
sudo npm install -g yarn
cd substrate-front-end-template; git checkout latest; yarn install; cd ..

# substrate linux contracts node
declare tempFile=temp-substrate-contracts-node
declare subConNodeUrl="https://github.com/paritytech/substrate-contracts-node/releases/download/v0.33.0/substrate-contracts-node-linux.tar.gz"
sudo curl -L ${subConNodeUrl} -o ${tempFile}".tar.gz"; \
    sudo gunzip ${tempFile}".tar.gz"; \
    sudo tar -xvf ${tempFile}".tar"; \
    sudo rm -fr ${tempFile}".tar"; \
    sudo mv artifacts/substrate-contracts-node-linux .;\
    sudo rmdir artifacts

cd

# configure cargo for smart contracts\
# fall back to 1.69 stable, instead of nightly for consistent
# contract builds
sudo ${CARGO_BIN}/rustup update stable
sudo ${CARGO_BIN}/rustup install 1.69
sudo ${CARGO_BIN}/rustup default 1.69
sudo ${CARGO_BIN}/rustup component add rust-src --toolchain 1.69
sudo ${CARGO_BIN}/rustup target add wasm32-unknown-unknown --toolchain 1.69

# install cargo for smart contract, based on
# https://docs.substrate.io/tutorials/smart-contracts/prepare-your-first-contract/
sudo cargo install --force --locked cargo-contract --version 2.0.0-rc

# install smart contract lint
sudo ${CARGO_BIN}/cargo install dylint-link

# install cargo contract, for smart contracts
sudo ${CARGO_BIN}/cargo install --force cargo-contract

# install glib for node addon/plugin support
sudo apt-get -y install libglib2.0-dev

# install polkadot standard binary, w/ latest related version
# to get release #, see: https://github.com/paritytech/polkadot/releases
declare POLKADOT_VERSION="v1.0.0"
sudo curl -sL https://github.com/paritytech/polkadot/releases/download/${POLKADOT_VERSION}/polkadot -o polkadot; \
    sudo chmod a+x polkadot; \
    sudo mv polkadot /usr/bin

# upon creation of the container run the startup script, and do any clean 
# up on the container, set time zone, copy bash_profile,
# then start bash

# change to root home dir
cd

# upon creation of the container run the startup script, and do any clean 
# # up on the container, set time zone, then start bash
# CMD \
#     # set timezone
#     ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone ; \
#     # run the startup script
#     chmod +x $STARTUP_DIR/ubuntu-start.sh ; \
#     $STARTUP_DIR/ubuntu-start.sh ; \
#     # start a bash shell
#     /bin/bash

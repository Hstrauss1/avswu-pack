!/bin/sh

# same/similar to Dockerfile

# syntax=docker/dockerfile:1
#
# docker file for creating hyperledger fabric ubuntu container
# #
# FROM ubuntu-dev

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
cd /root
sudo apt update
sudo apt install -y clang llvm
# rust
sudo curl https://sh.rustup.rs -sSf | sh -s -- -y

sudo echo 'source $HOME/.cargo/env' >> $HOME/.bashrc

# add cargo bin
declare CARGO_BIN="/root/.cargo/bin"

# update and configure rust and wasm
sudo ${CARGO_BIN}/rustup default stable; ${CARGO_BIN}/rustup update; ${CARGO_BIN}/rustup update nightly
sudo ${CARGO_BIN}/rustup target add wasm32-unknown-unknown --toolchain nightly
sudo ${CARGO_BIN}/rustup show; ${CARGO_BIN}/rustup +nightly show

# bindgen for creating rust to c bindings
sudo apt-get -y install llvm-dev libclang-dev clang
sudo ${CARGO_BIN}/cargo install bindgen-cli

# upon creation of the container sudo the startup script, and do any clean 
# up on the container, set time zone, copy bash_profile,
# then start bash

# change to root home dir
WORKDIR /root

# upon creation of the container sudo the startup script, and do any clean 
# up on the container, set time zone, then start bash
# CMD \
#     # set timezone
#     ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone ; \
#     # sudo the startup script
#     chmod +x $STARTUP_DIR/ubuntu-start.sh ; \
#     $STARTUP_DIR/ubuntu-start.sh ; \
#     # start a bash shell
#     /bin/bash

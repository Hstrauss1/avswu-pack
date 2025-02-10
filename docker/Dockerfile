# syntax=docker/dockerfile:1
#
# docker file for creating hyperledger fabric ubuntu container
#
FROM polkadot-grpc-dev

# environment variables
ARG USER="root"
ARG HOME="/root"

# container startup scripts
ARG STARTUP="startup"
ENV STARTUP_DIR=$HOME/$STARTUP

# copy bash startup script
ADD $STARTUP $STARTUP_DIR

# expose ports for ssh
EXPOSE 22
# expose ports for tightvncserver
# tightvncserver runs display 1 on port 5901 and display 2 on port 5902, et cetera
EXPOSE 5901

# create a volume to share data between host and other containers
VOLUME /shared-volume

#
# specific for this image
#

# GNU Multiple Precision Arithmetic Library, https://gmplib.org
RUN apt-get -y install m4 lzip xz-utils
ARG GMP_URL="https://gmplib.org/download/gmp/gmp-6.3.0.tar.xz"
ARG GMP_VERSION="gmp-6.3.0"
RUN curl -L $GMP_URL -o ${GMP_VERSION}".tar.xz"; \
    xz -d -v ${GMP_VERSION}".tar.xz"; \
    tar -xvf ${GMP_VERSION}".tar"; \
    rm -f ${GMP_VERSION}".tar"; \
    mv ${GMP_VERSION} /usr/local
WORKDIR /usr/local/${GMP_VERSION}
RUN ./configure
RUN make; make install

# PBC, stanford's pbc, pair based cryptography, https://crypto.stanford.edu/pbc/howto.html
# PBC is required for CPABE
RUN apt-get -y install flex bison
ARG PBC_URL="https://crypto.stanford.edu/pbc/files/pbc-0.5.14.tar.gz"
ARG PBC_VERSION="pbc-0.5.14"
RUN curl -L $PBC_URL -o ${PBC_VERSION}".tar.gz"; \
    gzip -d ${PBC_VERSION}".tar.gz"; \
    tar -xvf ${PBC_VERSION}".tar"; \
    rm -f ${PBC_VERSION}".tar"; \
    mv ${PBC_VERSION} /usr/local
WORKDIR /usr/local/${PBC_VERSION}
RUN ./configure
RUN make; make install

# CPABE libraries, http://acsc.cs.utexas.edu/cpabe
RUN apt-get -y install libglib2.0-dev libgtk2.0-dev

# CPABE libbswabe
# cpabe shared library run-time load path for .so files
ENV LD_LIBRARY_PATH="/usr/local/lib"
ARG CPABE_URL="http://acsc.cs.utexas.edu/cpabe/libbswabe-0.9.tar.gz"
ARG CPABE_VERSION="libbswabe-0.9"
RUN curl -L ${CPABE_URL} -o ${CPABE_VERSION}".tar.gz"; \
    gzip -d ${CPABE_VERSION}".tar.gz"; \
    tar -xvf ${CPABE_VERSION}".tar"; \
    rm -f ${CPABE_VERSION}".tar"; \
    mv ${CPABE_VERSION} /usr/local
WORKDIR /usr/local/${CPABE_VERSION}
RUN ./configure CFLAGS="-O3 -Wall -fPIC"
RUN make; make install

# cpp-json libraries
ARG CPP_JSON="json-develop"
ARG CPP_JSON_URL="https://github.com/nlohmann/json/archive/refs/heads/develop.zip"
RUN curl -L $CPP_JSON_URL -o ${CPP_JSON}".zip"; \
    unzip ${CPP_JSON}".zip"; \
    rm -f ${CPP_JSON}".zip"; \
    mv ${CPP_JSON} /usr/local
WORKDIR /usr/local/${CPP_JSON}
RUN cmake .
RUN make; make install

# spdlog c++ logging tool
WORKDIR /usr/local
RUN git clone https://github.com/gabime/spdlog.git
WORKDIR /usr/local/spdlog/build
RUN cmake .. && make -j; make install

# GF-Complete, required Jerasure
RUN git clone https://github.com/ceph/gf-complete; \
    mv gf-complete /usr/local
WORKDIR /usr/local/gf-complete
RUN ./autogen.sh ; ./configure; make; make install


# Jerasure
RUN git clone https://github.com/tsuraan/Jerasure; \
    mv Jerasure /usr/local
WORKDIR /usr/local/Jerasure
ENV LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/usr/local/lib:/usr/local/gf-complete/src/.libs"
RUN autoreconf --force --install; ./configure; make; make install

# GoogleTest framework
ARG GOOGLE_TEST_VERSION="1.14.0"
ARG GOOGLE_TEST="googletest-"${GOOGLE_TEST_VERSION}""
ARG GOOGLE_TEST_URL="https://github.com/google/googletest/archive/refs/tags/v"${GOOGLE_TEST_VERSION}".tar.gz"
RUN curl -L $GOOGLE_TEST_URL -o ${GOOGLE_TEST}".tar.gz"; \
    gzip -d ${GOOGLE_TEST}".tar.gz"; \
    tar -xvf ${GOOGLE_TEST}".tar"; \
    rm -f ${GOOGLE_TEST}".tar"; \
    mv ${GOOGLE_TEST} /usr/local
WORKDIR /usr/local/${GOOGLE_TEST}
RUN cmake .
RUN make; make install

# install clang++ 17
WORKDIR /usr/local
RUN wget https://apt.llvm.org/llvm.sh; \ 
    chmod u+x ./llvm.sh; \
    ./llvm.sh 17; \
    rm ./llvm.sh

# ipfs C++ go wrapper lib, https://github.com/scala-network/libipfs
# compiler for go

# install go, libipfs prereq
RUN apt-get -y update ; \
    apt-get -y remove golang-go
ARG GO_URL="https://go.dev/dl/go1.19.2.linux-amd64.tar.gz"
ARG GO_VERSION="go1.19.2"
RUN curl -L $GO_URL -o ${GO_VERSION}".tar.gz"; \
    gzip -d ${GO_VERSION}".tar.gz"; \
    tar -xvf ${GO_VERSION}".tar"; \
    rm -f ${GO_VERSION}".tar"; \
    mv go /usr/local
WORKDIR /usr/local/go

# libipfs
WORKDIR /usr/local
RUN git clone https://github.com/scala-network/libipfs
WORKDIR /usr/local/libipfs
RUN go mod download
RUN make build_linux_x64

# change to root home dir
WORKDIR /root

# upon creation of the container run the startup script, and do any clean 
# up on the container, set time zone, then start bash
CMD \
    # set timezone
    ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone ; \
    # run the startup script
    chmod +x $STARTUP_DIR/ubuntu-start.sh ; \
    $STARTUP_DIR/ubuntu-start.sh ; \
    # start a bash shell
    /bin/bash

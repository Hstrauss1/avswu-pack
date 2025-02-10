#!/bin/bash

# install script, based on https://veins.car2x.org/tutorial/
# versions  to install for veins 5.2 compatibility https://veins.car2x.org/download/#compatibility
#
# veins 5.2
# sumo v 1.11.0
# omnet v5.7
# inet 4.2.2

# veins must be installed locally under the users account (~/src directory)#!/bin/bash

# Define the installation directory to always be under the user's home directory
VEINS_INSTALL_DIR="$HOME/src"

# Ensure the directory exists
mkdir -p "$VEINS_INSTALL_DIR"
cd "$VEINS_INSTALL_DIR"

echo "------------------------------------------------------------------"
echo "STATUS: Installing Veins and dependencies in $VEINS_INSTALL_DIR"
echo "------------------------------------------------------------------"

# Install dependencies
sudo apt-get update
sudo apt-get install -y build-essential gcc g++ bison flex perl \
    tcl-dev tk-dev blt libxml2-dev zlib1g-dev default-jre doxygen graphviz \
    libwebkitgtk-1.0-0 openmpi-bin libopenmpi-dev libpcap-dev autoconf \
    automake libtool libproj-dev libgdal-dev libxerces-c-dev qtchooser \
    qtcreator qtbase5-dev qt5-qmake cmake clang-format unzip curl

# Install SUMO
echo "------------------------------------------------------------------"
echo "STATUS: Installing SUMO"
echo "------------------------------------------------------------------"
SUMO_VERSION="sumo-1.11.0"
curl -L "https://sourceforge.net/projects/sumo/files/sumo/version%201.11.0/${SUMO_VERSION}.orig.tar.gz" -o "${SUMO_VERSION}.tar.gz"
tar -xzf "${SUMO_VERSION}.tar.gz"
rm "${SUMO_VERSION}.tar.gz"
mv "${SUMO_VERSION}" "$VEINS_INSTALL_DIR"

# Install OMNeT++
echo "------------------------------------------------------------------"
echo "STATUS: Installing OMNeT++"
echo "------------------------------------------------------------------"
OMNETPP_VERSION="omnetpp-5.7"
curl -L "https://github.com/omnetpp/omnetpp/releases/download/omnetpp-5.7/${OMNETPP_VERSION}-src-linux.tgz" -o "${OMNETPP_VERSION}.tar.gz"
tar -xzf "${OMNETPP_VERSION}.tar.gz"
rm "${OMNETPP_VERSION}.tar.gz"
mv "${OMNETPP_VERSION}" "$VEINS_INSTALL_DIR"
cd "$VEINS_INSTALL_DIR/$OMNETPP_VERSION"
source setenv
sed -i 's/WITH_OSGEARTH=yes/WITH_OSGEARTH=no/g' configure.user
export PATH="$VEINS_INSTALL_DIR/$OMNETPP_VERSION/bin:$PATH"
./configure
make
make install

# Install Veins
echo "------------------------------------------------------------------"
echo "STATUS: Installing Veins"
echo "------------------------------------------------------------------"
VEINS_VERSION="veins-5.2"
curl -L "https://veins.car2x.org/download/${VEINS_VERSION}.zip" -o "${VEINS_VERSION}.zip"
unzip "${VEINS_VERSION}.zip"
rm "${VEINS_VERSION}.zip"
mv "${VEINS_VERSION}" "$VEINS_INSTALL_DIR/veins"
cd "$VEINS_INSTALL_DIR/veins"
./configure
make

# Install Cargo Contract
echo "------------------------------------------------------------------"
echo "STATUS: Installing Cargo Contract"
echo "------------------------------------------------------------------"
cargo install --force --locked cargo-contract --version 2.0.0-rc

echo "------------------------------------------------------------------"
echo "STATUS: Installation Complete"
echo "------------------------------------------------------------------"

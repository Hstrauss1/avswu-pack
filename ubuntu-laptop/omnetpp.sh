VEINS_INSTALL_DIR="$HOME/src"
sudo add-apt-repository -y universe
# Ensure the directory and dependancies exist
sudo apt install -y bison
sudo apt install -y flex
sudo apt install -y cmake
sudo apt install -y curl
sudo apt install -y build-essential
sudo apt install -y python3
sudo apt install -y npm
sudo apt install -y openjdk-8-jdk
sudo apt install -y build-essential gcc g++ bison flex perl \
    tcl-dev tk-dev blt libxml2-dev zlib1g-dev default-jre doxygen graphviz \
    libwebkitgtk-1.0-0 openmpi-bin libopenmpi-dev libpcap-dev autoconf \
    automake libtool libproj-dev libgdal-dev libxerces-c-dev qtchooser \
    qtcreator qtbase5-dev qt5-qmake cmake clang-format unzip curl
mkdir -p "$VEINS_INSTALL_DIR"
cd "$VEINS_INSTALL_DIR"

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
sed -i 's/WITH_OSG=yes/WITH_OSG=no/g' configure.user
sed -i 's/WITH_QTENV=yes/WITH_QTENV=no/g' configure.user
sed -i 's/PREFER_QTENV=yes/PREFER_QTENV=no/g' configure.user
sed -i 's/#CXXFLAGS=-std=c++17/CXXFLAGS=-std=c++17/g' configure.user
export PATH="$VEINS_INSTALL_DIR/$OMNETPP_VERSION/bin:$PATH"
./configure
make
make install


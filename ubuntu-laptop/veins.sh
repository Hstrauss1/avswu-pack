VEINS_INSTALL_DIR="$HOME/src"

sudo apt-get install -y build-essential
sudo apt-get install -y gcc
sudo apt-get install -y g++
sudo apt-get install -y bison flex perl
sudo apt-get install -y tcl-dev tk-dev blt
sudo apt-get install -y libxml2-dev zlib1g-dev
sudo apt-get install -y default-jre doxygen graphviz
sudo apt-get install -y libwebkitgtk-1.0-0 openmpi-bin
sudo apt-get install -y libopenmpi-dev libpcap-dev autoconf
sudo apt-get install -y automake libtool libproj-dev
sudo apt-get install -y libgdal1-dev libfox-1.6-dev
sudo apt-get install -y libgdal-dev libxerces-c-dev
sudo apt-get install -y qt4-dev-tools
sudo apt-get -y install curl
# Install dependencies
sudo apt-get update
sudo apt-get install -y build-essential gcc g++ bison flex perl \
    tcl-dev tk-dev blt libxml2-dev zlib1g-dev default-jre doxygen graphviz \
    libwebkitgtk-1.0-0 openmpi-bin libopenmpi-dev libpcap-dev autoconf \
    automake libtool libproj-dev libgdal-dev libxerces-c-dev qtchooser \
   
sudo apt install -y qtcreator qtbase5-dev qt5-qmake cmake clang-format unzip curl

# Ensure the directory exists
mkdir -p "$VEINS_INSTALL_DIR"
cd "$VEINS_INSTALL_DIR"/

echo "------------------------------------------------------------------"
echo "STATUS: Installing Veins"
echo "------------------------------------------------------------------"
VEINS_VERSION="veins-5.2"
curl -L "https://veins.car2x.org/download/${VEINS_VERSION}.zip" -o "${VEINS_VERSION}.zip"
unzip "${VEINS_VERSION}.zip"
rm "${VEINS_VERSION}.zip"
mv "${VEINS_VERSION}" "$VEINS_INSTALL_DIR"
cd "$VEINS_INSTALL_DIR/veins-$VEINS_VERSION"
cd "$HOME/src/veins-veins-5.2"
source setenv
./configure
make

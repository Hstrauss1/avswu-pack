export SUMO_URL="https://sourceforge.net/projects/sumo/files/sumo/version%201.11.0/sumo-1.11.0.orig.tar.gz"
export SUMO_VERSION="sumo-1.11.0"
sudo add-apt-repository universe
sudo apt update
sudo apt-get install -y build-essential gcc g++ bison flex perl \
    tcl-dev tk-dev blt libxml2-dev zlib1g-dev default-jre doxygen graphviz \
    libwebkitgtk-1.0-0 openmpi-bin libopenmpi-dev libpcap-dev autoconf \
    automake libtool libproj-dev libgdal-dev libxerces-c-dev qtchooser \
    qtcreator qtbase5-dev qt5-qmake cmake clang-format unzip curl
sudo apt install -y libgtest-dev
sudo apt install -y python3-setuptools
sudo apt-get install -y zip unzip tar python3 nodejs npm xfce4-terminal tree jq dos2unix iputils-ping busybox openjdk-8-jdk
#set(Python_ADDITIONAL_VERSIONS 3.7 3.6 3.5 3.4 3.3 2.7)
sudo apt install -y maven
sudo apt install -y libopenscenegraph-dev
sudo apt install -y build-essential cmake g++ libeigen3-dev
sudo apt install -y python3-dev python3-pip
sudo apt install -y gcc-12 g++-12

echo "Downloading SUMO..."
curl -L "https://sourceforge.net/projects/sumo/files/sumo/version%201.11.0/sumo_1.11.0.orig.tar.gz/download" -o "sumo-1.11.0.tar.gz"

# Wait 5 seconds to ensure the download is fully completed
sleep 7
tar -xzf "sumo-1.11.0.tar.gz"
rm "sumo-1.11.0.tar.gz"
mkdir "$HOME/src"
mv "sumo-1.11.0" "$HOME/src"

sudo apt install cmake python3 g++ libxerces-c-dev libfox-1.6-dev libgdal-dev libproj-dev libgl2ps-dev swig

cd "$HOME/src/$SUMO_VERSION" 
sed -i 's/std=c++11/std=c++14/g' CmakeLists.txt
cmake .
make 
make install

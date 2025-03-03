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

#!/bin/bash

echo "------------------------------------------------------------------"
echo "STATUS: Starting full partial Ubuntu setup..."
echo "------------------------------------------------------------------"

# Ensure scripts are executable
chmod +x *.sh
sudo apt update && sudo apt install -y cmake
# Run each setup script in order
echo "Running omnetpp.sh..."
./omnetpp.sh 

echo "Running veins.sh..."
./veins.sh 

echo "Running sumo.sh..."
./sumo.sh

pip install compiledb
sudo apt install nlohmann-json3-dev
sudo apt install libgmp-dev -y
sudo apt install libspdlog-dev -y

cd ~/avswu/veins-client/veins_avswu
cp ~/avswu/veins-server/veins_avswu.proto .
protoc \
  -I ~/avswu/veins-server \
  --cpp_out=. \
  --grpc_out=. \
  --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) \
  veins_avswu.proto

echo "------------------------------------------------------------------"
echo "STATUS: All setup scripts completed successfully!"
echo "------------------------------------------------------------------"

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

echo "------------------------------------------------------------------"
echo "STATUS: All setup scripts completed successfully!"
echo "------------------------------------------------------------------"

#!/bin/bash

echo "------------------------------------------------------------------"
echo "STATUS: Starting full Ubuntu setup..."
echo "------------------------------------------------------------------"

# Ensure scripts are executable
chmod +x *.sh
sudo apt update && sudo apt install -y cmake
# Run each setup script in order
echo "Running ubuntu-dev.sh..."
#./ubuntu-dev.sh || { echo "ubuntu-dev.sh failed! Exiting."; exit 1; }

echo "Running setup-laptop.sh..."
./setup-laptop.sh || { echo "setup-laptop.sh failed! Exiting."; exit 1; }

echo "Running install-veins.sh..."
./install-veins.sh || { echo "install-veins.sh failed! Exiting."; exit 1; }


echo "Running polkadot-dev.sh..."
./polkadot-dev.sh || { echo "polkadot-dev.sh failed! Exiting."; exit 1; }

echo "Running polkadot-grpc-dev.sh..."
./polkadot-grpc-dev.sh || { echo "polkadot-grpc-dev.sh failed! Exiting."; exit 1; }
echo "Running rust-dev.sh..."
./rust-dev.sh || { echo "rust-dev.sh failed! Exiting."; exit 1; }

echo "Running ubuntu-start.sh..."
./ubuntu-start.sh || { echo "ubuntu-start.sh failed! Exiting."; exit 1; }

echo "------------------------------------------------------------------"
echo "STATUS: All setup scripts completed successfully!"
echo "------------------------------------------------------------------"

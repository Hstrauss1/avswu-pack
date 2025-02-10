#!/bin/bash

echo "------------------------------------------------------------------"
echo "STATUS: Starting..."
currentDate=`date`
echo "Date = " $currentDate
echo "------------------------------------------------------------------"

echo "------------------------------------------------------------------"
echo "STATUS:       Starting substrate contract node..."
echo "------------------------------------------------------------------"

# web socket port defaults to 9944
cd /usr/local/substrate/substrate-contracts-node-linux
./substrate-contracts-node --log info,runtime::contracts=debug 2>&1 --rpc-cors all --unsafe-rpc-external --name test-node --dev

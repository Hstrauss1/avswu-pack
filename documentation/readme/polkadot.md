# working with substrate files

1. most commands run within docker container. so run commands from ubuntu
2. when copying substrate templates, remove the .git folder to prevent git errors (from nested git repos) within our main repo

# how to start the front-end

The Substrate front-end consists of user interface components to enable you to interact with the Substrate node and perform a few common tasks.

1. Open a new terminal shell on your computer, change to the root directory where you installed the front-end template.

   > docker exec -it avswu /bin/bash
   >
   > cd ~/avswu/substrate-smart-contract-front-end

2. Start the Front-end by running the following command:

> npm start

3.  start new shell and open browser

> open browser on windows to: http://127.0.0.1:8000/substrate-smart-contract-front-end

# to run tightvnc and vnc into a container

to get the ip address of the container?

> docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' polka

open tigervnc client on windows to: 127.0.0.1:5951
password is: welcome

# if vnc fails to run

on linux cleanup vncserver files

remove xserver files, kill the server, and restart it

```bash
ps -ef | grep vnc
kill -9 VNC_PROCESS_NUMBER
rm -fr /root/.vnc
rm -fr /tmp/.X*
tightvncserver -geometry 1920x1080
```

# ink smart contract tutorial

https://docs.substrate.io/tutorials/smart-contracts/prepare-your-first-contract/

Note: all smart contract configuration and compilation steps (from tutorial) are in the Dockerfile

## build the smart contracts

> cd ~/avswu/smart_contract_flipper
>
> smart-contract-setup.sh

## 5.9 build the iot-contract-node

> cd ~/avswu/substrate-smart-contract-node

> cargo +nightly test

> cargo +nightly build

## 6. start the **contracts** node

> cd ~/avswu/substrate-smart-contract-node

> ./target/debug/iot-contract-node --ws-port 9944 --rpc-port 9933 --rpc-cors all --ws-external --rpc-external --prometheus-external --base-path /tmp/test-node --name test-node --dev

## 7. start the parity UI to monitor block creation and upload contracts as/if needed

select Local Node

> https://paritytech.github.io/contracts-ui/#/

## 8. upload the contract code

via steps in webpage

8.1 Open to the Contracts UI in a web browser.
8.2 Verify that you are connected to the Local Node.
8.3 Click Add New Contract.
8.4 Click Upload New Contract .contract code located in
C:\Users\jerom\Documents\GitHub\avswu\smart_contract_flipper\target\ink

8.5 Select an Account to use to create a contract instance.
8.6 You can select any existing account, including a predefined account such as alice.
8.7 Type a descriptive Name for the smart contract, for example, Flipper Contract.
8.8 Browse and select or drag and drop the flipper.contract file that contains the bundled Wasm blob and metadata into the upload section.

## (optional) run front-end

> cd ~/avswu/substrate-smart-contract-front-end
> yarn start

then open browser to:

> http://127.0.0.1:8000/substrate-smart-contract-front-end

MAYBE USE subkey and install contract with cargo

# how to use subkey tool

alice address: 5GrwvaEF5zXb26Fz9rcQpDWS57CtERHpNehXCPcNoHGKutQY

> subkey inspect 5GrwvaEF5zXb26Fz9rcQpDWS57CtERHpNehXCPcNoHGKutQY

> cargo contract upload target/ink/smart_contract_flipper.contract --suri NEED_SURI_HERE

Alice's secret key can be found with subkey

> subkey inspect //Alice

more info on subkey:

> https://substrate.dev/docs/en/knowledgebase/integrate/subkey#well-known-keys

# polkadot advanced blockchain explorer tool

switch to chain or local node in uppper left part of UI

> https://polkadot.js.org/apps/?rpc=ws%3A%2F%2F127.0.0.1%3A9944#/explorer

# substrate's contract node (for smart contract development)

> https://github.com/paritytech/substrate-contracts-node

# how to extend a node to support smart contract

1. add the contracts pallet

How to add contracts pallet

> https://docs.substrate.io/how-to-guides/v3/pallet-design/contracts-pallet/

contracts pallet docs

> https://paritytech.github.io/substrate/master/pallet_contracts/index.html

susbtrate contract node source code

> https://github.com/paritytech/substrate-contracts-node

### test the smart contract (on linux container)

```bash
cd ~/avswu/contract_notification
cargo test -- --nocapture
```

### if needed, build the contract artifacts (on linux container)

```bash
cd ~/avswu/contract_notification
cargo contract build --release
cd ~/avswu/contract_commitment
cargo contract build --release
```

# to run the polkadot front end

## 1. start the front end (on windows)

> cd-avswu
> cd .\avswu-dapp
> npm start

## 2. open a browser with contracts UI

select Local Node

> https://paritytech.github.io/contracts-ui/#/

## 3. upload the contract code

via steps in webpage

3.1 Open to the Contracts UI in a web browser.
3.2 Verify that you are connected to the Local Node.
3.3 Click Add New Contract.
3.4 Click Upload New Contract Code.
3.5 Select an Account to use to create a contract instance.
3.6 You can select any existing account, including a predefined account such as alice.
3.7 Type a descriptive Name for the smart contract, for example, Flipper Contract.
3.8 Browse and select or drag and drop the flipper.contract file that contains the bundled Wasm blob and metadata into the upload section. From

> C:\Users\jerom\Documents\GitHub\avswu\smart_contract_flipper\target\ink

# dapp dev

## how to build a release

> cd ~/avswu/substrate-smart-contract-node
> cargo build --release

## how to run tests

how to run watched tests that run upon changes (like jest test runner)

> cargo watch -x test

# polkadot node types

> https://wiki.polkadot.network/docs/maintain-sync

## archive node

- manufacturer = archive-node?
- takes up more space, keeps all transaction

## full node

- A full node is pruned: it discards all finalized blocks older than a configurable
- number except the genesis block.
- keeps by default 256 blocks

## light node

av's

- A light node has only the runtime and the current state
- does not store past blocks and so cannot read historical data without requesting it from a node that has it.
- Light nodes are useful for resource restricted devices.

## validator node

- A blockchain's growth comes from a genesis block, extrinsics, and events.
- once two thirds of the validators agree on a specific block being valid, it is finalized.
- When a validator seals block 1, it takes the blockchain's state at block 0. It then applies all pending changes on top of it, and emits the events that are the result of these changes.

# running a node on westend testnet

## westend test network

> polkadot --chain westend --name "gsolo-campbell1"

watch on polkadot's telemetry website

add westend to the polkadot chains (upper right corner of chrome)

> https://telemetry.polkadot.io/#list/0xe143f23803ac50e8f6f8e62695d1ce9e4e1d68aa36c1cd2cfd15340213f3423e

## for cloud hosted nodes, need to set up certificate

> https://wiki.polkadot.network/docs/maintain-wss

## to run a light node

> ./target/release/polkadot --light

## to run a validator node

./target/release/polkadot --validator

## running an archive node

> ./target/release/polkadot --pruning archive

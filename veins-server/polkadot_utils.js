import process from 'node:process';
import logger from './logger_utils.js';

import { execSync } from 'child_process';

// load polkadot and grpc
logger.debug('loading polkadot...');
import { ApiPromise, WsProvider, Keyring } from '@polkadot/api';
import { ContractPromise } from '@polkadot/api-contract';
import { u8aToString } from '@polkadot/util';
import fs from 'fs';
import { BN, BN_ONE } from '@polkadot/util';

/*
 * veins -> avswu blockchain
 */

// get contract promise
const getContract = (api, contractDir, contractFile, contractAddress) => {
  // read metadata (abi)
  const contractMetadataFile = contractDir + '/target/ink/' + contractFile;
  const metadataJson = fs.readFileSync(contractMetadataFile);
  const metadataJsonStr = u8aToString(metadataJson);

  // set the global contract variable
  const contract = new ContractPromise(api, metadataJsonStr, contractAddress);

  return contract;
};

// connect to blockchain
const blockchainConnect = async () => {
  const provider = new WsProvider('ws://127.0.0.1:9944');
  const api = await ApiPromise.create({ provider });

  // alice account
  const keyring = new Keyring({ type: 'sr25519' });
  const alicePair = keyring.addFromUri('//Alice', { name: 'Alice default' });

  const accountPair = alicePair;

  return { api, accountPair };
};

// figure out gasLimit per
// https://substrate.stackexchange.com/questions/6401/smart-contract-function-call-error
const maxGasLimit = (api) => {
  const MAX_CALL_WEIGHT = new BN(5_000_000_000_000).isub(BN_ONE);
  const PROOFSIZE = new BN(1_000_000);
  const gasLimitQuery = api.registry.createType('WeightV2', {
    refTime: MAX_CALL_WEIGHT,
    proofSize: PROOFSIZE
  });

  return gasLimitQuery;
};

// get contract address
const deployContract = (contractDir) => {
  let addr;
  // check command line
  const argv = process.argv;
  if (argv.length == 3) {
    addr = argv[2];
  } else {
    // deploy the contract, and get the adddress
    logger.debug('deploying the contract contractDir = %s', contractDir);
    const cmd =
      'cd ' +
      contractDir +
      ';' +
      // eslint-disable-next-line max-len
      'yes | cargo contract instantiate --constructor new --suri //Alice --salt $(date +%s) | grep "Contract " | awk \'{ print $(NF) }\';';
    try {
      addr = execSync(cmd).toString();
      // remove \n from string
      addr = addr.replace('\n', '');
      if (addr.length == 0) {
        logger.fatal(
          'No contract address. Make sure the node is reachable and running. Contract not deployed.'
        );
        process.exit(-1);
      }
    } catch (err) {
      if (err) {
        // node couldn't execute the command
        logger.fatal(
          'Unable to deploy the contract. Make sure the node is reachable and running. error = %s',
          err
        );
        process.exit(-1);
      }
    }
  }
  return addr;
};

// get result data from contract
const parseContractResult = (output) => {
  let dataValue = undefined;
  // get the output
  const human = output.toHuman();
  if (human.Ok != null) {
    const data = human.Ok;
    // if get failed
    if (data.Err) {
      dataValue = data;
      logger.warn(
        'unable to get contract results. the results may not yet be available on blockchain'
      );
      logger.debug('data.Err = %s', JSON.stringify(data.Err, null, 2));
    }
    // if data contains an ok, get the value
    if (data.Ok) {
      dataValue = data.Ok;
    }
    return dataValue;
  } else {
    logger.fatal(
      'unable to get smart contract results human.Ok = %s',
      human.Ok
    );
  }
};

export {
  deployContract,
  getContract,
  maxGasLimit,
  blockchainConnect,
  parseContractResult
};

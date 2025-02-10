import { maxGasLimit, parseContractResult } from './polkadot_utils.js';
import logger from './logger_utils.js';

// get the gas required
const callReadCommitmentVector = async (request, bcConnection, contract) => {
  const msgPrefix = 'callReadCommitmentVector: ';
  const { api, accountPair } = bcConnection;
  const storageDepositLimit = null;
  const gasLimit = maxGasLimit(api);

  // find out how much gas to execute the call based on method
  // eslint-disable-next-line camelcase
  const { key_vec_json } = request;

  // eslint-disable-next-line camelcase
  if (key_vec_json && key_vec_json.length == 0) {
    logger.error(
      msgPrefix + 'map key vector sent to read_commitment_vector is empty'
    );
    return;
  }

  // convert to utf8 string to send to smart contract
  // eslint-disable-next-line camelcase
  const keyVecJsonUtf8 = key_vec_json.toString('utf8');

  const { result, output } = await contract.query.readCommitmentVector(
    accountPair.address,
    {
      gasLimit,
      storageDepositLimit
    },
    keyVecJsonUtf8
  );

  return { result, output };
};

// call smart contract to write a commitment
async function avswuBlockchainReadCommitmentVector(
  call,
  callback,
  bcConnection,
  contract
) {
  const msgPrefix = 'avswuBlockchainReadCommitmentVector: ';
  // get gas required to execute smart contract, and the output
  logger.info(
    msgPrefix +
      'executing read commitment vector: getting smart contract result'
  );
  const { result, output } = await callReadCommitmentVector(
    call.request,
    bcConnection,
    contract
  );

  let contractResult;
  try {
    contractResult = parseContractResult(output);
  } catch (error) {
    const message = msgPrefix + 'unable to parse the result of smart contract';
    const errorJson = { error, message };
    logger.fatal(JSON.stringify(errorJson), null, 2);
    // send back the error
    callback(null, { result: JSON.stringify(errorJson) });
    return;
  }

  // get smart contract results
  if (result.isOk) {
    // send contract result back to client over grpc
    const resultJson = JSON.stringify(contractResult);
    callback(null, { result: resultJson });
    return;
  } else {
    const error = JSON.stringify(result.asErr, null, 2);
    const message = msgPrefix + 'unable to get result of smart contract';
    const errorJson = { error, message };
    logger.fatal(JSON.stringify(errorJson), null, 2);
    // send back the error
    callback(null, { result: JSON.stringify(errorJson) });
    return;
  }
}

export { avswuBlockchainReadCommitmentVector };

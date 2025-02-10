import { maxGasLimit, parseContractResult } from './polkadot_utils.js';
import logger from './logger_utils.js';

const callWriteCommitment = async (
  gasRequired,
  request,
  bcConnection,
  contract
) => {
  const { api, accountPair } = bcConnection;
  const storageDepositLimit = null;
  const gasLimitTx = api.registry.createType('WeightV2', gasRequired);

  // assume the tx fails, then prove that its successful
  let txSuccess = false;

  // execute the call
  logger.debug('executing the write_commitment smart contract call');
  // eslint-disable-next-line camelcase
  const { shard_id, signer_id, capital_r, w, capital_w } = request;

  // eslint-disable-next-line no-unused-vars
  const unsubscribe = await contract.tx
    .writeCommitment(
      {
        gasLimit: gasLimitTx,
        storageDepositLimit
      },
      shard_id,
      signer_id,
      capital_r,
      w,
      capital_w
    )
    .signAndSend(accountPair, (result) => {
      const { txHash, status } = result;
      if (status.isInBlock) {
        logger.debug(
          'contract executed. tx is in a block. tx hash = %s',
          txHash.toString()
        );
        // if successful, send back the results of contract executation
        // over gRPC
        txSuccess = true;
      } else if (status.isFinalized) {
        logger.debug(
          'contract executed. tx is finalized. tx hash = %s',
          txHash.toString()
        );
      }
    });

  return txSuccess;
};

// get the gas required
const gasRequiredAndOutputWriteCommitment = async (
  request,
  bcConnection,
  contract
) => {
  const { api, accountPair } = bcConnection;
  const storageDepositLimit = null;
  const gasLimit = maxGasLimit(api);

  // find out how much gas to execute the call based on method
  // eslint-disable-next-line camelcase
  const { shard_id, signer_id, capital_r, w, capital_w } = request;

  const { result, gasRequired, output } = await contract.query.writeCommitment(
    accountPair.address,
    {
      gasLimit,
      storageDepositLimit
    },
    shard_id,
    signer_id,
    capital_r,
    w,
    capital_w
  );

  return { result, gasRequired, output };
};

// call smart contract to write a commitment
async function avswuBlockchainWriteCommitment(
  call,
  callback,
  bcConnection,
  contract
) {
  const msgPrefix = 'avswuBlockchainWriteCommitment: ';
  // get gas required to execute smart contract, and the output
  logger.debug(
    msgPrefix +
      'determining gas required to execute smart contract, and getting output'
  );
  const { result, gasRequired, output } =
    await gasRequiredAndOutputWriteCommitment(
      call.request,
      bcConnection,
      contract
    );

  // if result was not received, return error
  if (!result.isOk) {
    const error = result.asErr;
    const message =
      msgPrefix + 'unable to get the result from smart contract method call';
    const errorJson = { error, message };
    logger.fatal(JSON.stringify(errorJson), null, 2);
    // send back the error
    callback(null, { result: JSON.stringify(errorJson) });
    return;
  }

  // parse smart contract results
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

  logger.debug(
    msgPrefix + 'smart contract return results dataValue = %s',
    JSON.stringify(contractResult, null, 2)
  );

  logger.info(
    msgPrefix +
      'executing the write commitment smart contract and commit changes in to a block'
  );
  const txSuccess = callWriteCommitment(
    gasRequired,
    call.request,
    bcConnection,
    contract
  );

  // if tx is not a success, warn user, but send back results so another call
  // can be attempted
  if (!txSuccess) {
    const error = { txSuccess };
    const message =
      msgPrefix + 'write notification smart contract method failed';
    const errorJson = { error, message };
    logger.fatal(JSON.stringify(errorJson), null, 2);
    // send back the error
    callback(null, { result: JSON.stringify(errorJson) });
    return;
  }

  // send successful transaction
  callback(null, { result: JSON.stringify(contractResult) });
}

export { avswuBlockchainWriteCommitment };

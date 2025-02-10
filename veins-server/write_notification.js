import { maxGasLimit, parseContractResult } from './polkadot_utils.js';
import logger from './logger_utils.js';

const callWriteNotification = async (
  gasRequired,
  request,
  bcConnection,
  contract
) => {
  const msgPrefix = 'callWriteNotification: ';
  const { api, accountPair } = bcConnection;
  const storageDepositLimit = null;
  const gasLimitTx = api.registry.createType('WeightV2', gasRequired);

  // logger.error('request=%s', JSON.stringify(request, null, 2));

  // assume the tx fails, then prove that its successful
  let txSuccess = false;

  // execute the call
  logger.debug(
    msgPrefix + 'executing the write_notification smart contract call'
  );
  // eslint-disable-next-line camelcase
  const {
    manufacturer_id,
    software_id,
    version_id,
    create_time,
    notification_json
  } = request;

  // eslint-disable-next-line no-unused-vars
  const unsubscribe = await contract.tx
    .writeNotification(
      {
        gasLimit: gasLimitTx,
        storageDepositLimit
      },
      manufacturer_id,
      software_id,
      version_id,
      create_time,
      notification_json
    )
    .signAndSend(accountPair, (result) => {
      const { txHash, status } = result;
      if (status.isInBlock) {
        logger.debug(
          msgPrefix + 'contract executed. tx is in a block. tx hash = %s',
          txHash.toString()
        );
        // if successful, send back the results of contract executation
        // over gRPC
        txSuccess = true;
      } else if (status.isFinalized) {
        logger.debug(
          msgPrefix + 'contract executed. tx is finalized. tx hash = %s',
          txHash.toString()
        );
      }
    });

  return txSuccess;
};

// get the gas required
const gasRequiredAndOutputWriteNotification = async (
  request,
  bcConnection,
  contract
) => {
  const { api, accountPair } = bcConnection;
  const storageDepositLimit = null;
  const gasLimit = maxGasLimit(api);

  // find out how much gas to execute the call based on method
  // eslint-disable-next-line camelcase
  const {
    manufacturer_id,
    software_id,
    version_id,
    create_time,
    notification_json
  } = request;

  const { result, gasRequired, output } =
    await contract.query.writeNotification(
      accountPair.address,
      {
        gasLimit,
        storageDepositLimit
      },
      manufacturer_id,
      software_id,
      version_id,
      create_time,
      notification_json
    );

  return { result, gasRequired, output };
};

// call smart contract to write a notification
async function avswuBlockchainWriteNotification(
  call,
  callback,
  bcConnection,
  contract
) {
  const msgPrefix = 'avswuBlockchainWriteNotification: ';
  // get gas required to execute smart contract, and the output
  if (call.request) {
    const {
      manufacturer_id,
      software_id,
      version_id,
      create_time,
      notification_json
    } = call.request;
    logger.info(
      msgPrefix +
        '(manufacturer_id=' +
        manufacturer_id +
        ', ' +
        'software_id=' +
        software_id +
        ', ' +
        'version_id=' +
        version_id +
        ', ' +
        'create_time=' +
        create_time +
        ')'
    );
  }
  logger.debug(
    msgPrefix +
      'determining gas required to execute smart contract, and getting output'
  );
  const { result, gasRequired, output } =
    await gasRequiredAndOutputWriteNotification(
      call.request,
      bcConnection,
      contract
    );

  // if result was not received, return an error
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

  logger.info(
    msgPrefix +
      'executing the write notification smart contract and commit changes in to a block'
  );
  const txSuccess = callWriteNotification(
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

export { avswuBlockchainWriteNotification };

import { maxGasLimit, parseContractResult } from './polkadot_utils.js';
import logger from './logger_utils.js';

// get the gas required
const callReadLastNotification = async (request, bcConnection, contract) => {
  const msgPrefix = 'callReadLastNotification: ';

  const { api, accountPair } = bcConnection;
  const storageDepositLimit = null;
  const gasLimit = maxGasLimit(api);

  // find out how much gas to execute the call based on method
  // eslint-disable-next-line camelcase
  const { manufacturer_id, software_id, start_time } = request;
  logger.debug(msgPrefix + 'request=%s', JSON.stringify(request), null, 2);

  const { result, output } = await contract.query.readLastNotification(
    accountPair.address,
    {
      gasLimit,
      storageDepositLimit
    },
    manufacturer_id,
    software_id,
    start_time
  );

  return { result, output };
};

// call smart contract to write a notification
async function avswuBlockchainReadLastNotification(
  call,
  callback,
  bcConnection,
  contract
) {
  const msgPrefix = 'avswuBlockchainReadLastNotification: ';
  // get gas required to execute smart contract, and the output
  logger.info(
    msgPrefix +
      'executing read last notification: getting smart contract result'
  );
  const { result, output } = await callReadLastNotification(
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

export { avswuBlockchainReadLastNotification };

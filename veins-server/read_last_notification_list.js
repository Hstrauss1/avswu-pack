import { maxGasLimit, parseContractResult } from './polkadot_utils.js';
import logger from './logger_utils.js';

// get the gas required
const callReadLastNotificationList = async (
  request,
  bcConnection,
  contract
) => {
  const msgPrefix = 'callReadLastNotificationList: ';
  const { api, accountPair } = bcConnection;
  const storageDepositLimit = null;
  const gasLimit = maxGasLimit(api);

  // find out how much gas to execute the call based on method
  // eslint-disable-next-line camelcase
  const { manufacturer_list_json, start_time } = request;

  // eslint-disable-next-line camelcase
  if (manufacturer_list_json && manufacturer_list_json.length == 0) {
    logger.error(msgPrefix + 'manufacturer_list json is empty');
    return;
  }

  // convert to utf8 string to send to smart contract
  // eslint-disable-next-line camelcase
  const manufacturerListUtf8 = manufacturer_list_json.toString('utf8');

  const { result, output } = await contract.query.readLastNotificationList(
    accountPair.address,
    {
      gasLimit,
      storageDepositLimit
    },
    manufacturerListUtf8,
    start_time
  );

  return { result, output };
};

function logReadResults(output) {
  const msgPrefix = 'logReadResults: ';
  try {
    const output_obj = JSON.parse(output);
    if (output_obj.ok && output_obj.ok.ok) {
      const arr = output_obj.ok.ok;
      for (let i = 0; i < arr.length; i++) {
        const notif = JSON.parse(arr[i].notificationJson);
        const { manufacturer_id, software_id, version_id, create_time } = notif;
        logger.info(
          msgPrefix +
            'result[' +
            i +
            ']=' +
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
    }
  } catch (error) {
    const message = msgPrefix + 'unable to log the results';
    const errorJson = { error, message };
    logger.fatal(JSON.stringify(errorJson), null, 2);
    // send back the error
    callback(null, { result: JSON.stringify(errorJson) });
    return;
  }
}

// call smart contract to write a commitment
async function avswuBlockchainReadLastNotificationList(
  call,
  callback,
  bcConnection,
  contract
) {
  const msgPrefix = 'avswuBlockchainReadLastNotificationList: ';
  if (call.request) {
    logger.info(
      msgPrefix + 'call.request=' + JSON.stringify(call.request, null, 2)
    );
  }

  // get gas required to execute smart contract, and the output
  const { result, output } = await callReadLastNotificationList(
    call.request,
    bcConnection,
    contract
  );

  // log read last results summary
  logReadResults(output);

  let contractResult;
  try {
    contractResult = parseContractResult(output);
  } catch (error) {
    const message =
      msgPrefix +
      'unable to parse the result of smart contract, result=' +
      JSON.stringify(result, null, 2);
    const errorJson = { error, message };
    logger.fatal(JSON.stringify(errorJson), null, 2);
    // send back the error
    callback(null, { result: JSON.stringify(errorJson) });
    return;
  }

  // logger.info(msgPrefix + 'result=' + JSON.stringify(result, null, 2));
  // logger.info(
  //   msgPrefix + 'contractResult=' + JSON.stringify(contractResult, null, 2)
  // );

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

export { avswuBlockchainReadLastNotificationList };

import process from 'node:process';
import logger from './logger_utils.js';

import {
  deployContract,
  blockchainConnect,
  getContract
} from './polkadot_utils.js';

const AVSWU_DIR = '/home/gsolomon/avswu';

logger.debug('loading grpc...');
// grpc globals
const PROTO_PATH = AVSWU_DIR + '/veins-server/veins_avswu.proto';

import grpc from '@grpc/grpc-js';
import protoLoader from '@grpc/proto-loader';
const packageDefinition = protoLoader.loadSync(PROTO_PATH, {
  keepCase: true,
  longs: String,
  enums: String,
  defaults: true,
  oneofs: true
});
const avswuProto = grpc.loadPackageDefinition(packageDefinition).avswu;

import { avswuBlockchainWriteCommitment } from './write_commitment.js';
import { avswuBlockchainReadCommitmentVector } from './read_commitment_vector.js';

import { avswuBlockchainWriteNotification } from './write_notification.js';
import { avswuBlockchainReadLastNotification } from './read_notification.js';
import { avswuBlockchainReadLastNotificationList } from './read_last_notification_list.js';

// blockchain connection (this needs to be global)
let bcConnection = null;

/*
 * veins -> blockchain
 */

const main = async () => {
  // connect to blockchain
  logger.debug('connecting to blockchain');
  bcConnection = await blockchainConnect();

  // commitment contract
  const contractDirCommitment = AVSWU_DIR + '/contract_commitment';
  const contractFileCommitment = 'metadata.json';

  const contractAddrCommitment = deployContract(contractDirCommitment);
  logger.debug('contractAddrCommitment=%s', contractAddrCommitment);

  const contractCommitment = getContract(
    bcConnection.api,
    contractDirCommitment,
    contractFileCommitment,
    contractAddrCommitment
  );

  // notification contract
  const contractDirNotification = AVSWU_DIR + '/contract_notification';
  const contractFilenNotification = 'metadata.json';

  const contractAddrNotification = deployContract(contractDirNotification);
  logger.debug('contractAddrCommitment=%s', contractAddrNotification);

  const contractNotification = getContract(
    bcConnection.api,
    contractDirNotification,
    contractFilenNotification,
    contractAddrNotification
  );

  // start the grpc server
  const server = new grpc.Server();

  // add services
  server.addService(avswuProto.Avswu.service, {
    avswuBlockchainWriteCommitment: async (call, callback) => {
      avswuBlockchainWriteCommitment(
        call,
        callback,
        bcConnection,
        contractCommitment
      );
    },
    avswuBlockchainReadCommitmentVector: async (call, callback) => {
      avswuBlockchainReadCommitmentVector(
        call,
        callback,
        bcConnection,
        contractCommitment
      );
    },
    avswuBlockchainWriteNotification: async (call, callback) => {
      avswuBlockchainWriteNotification(
        call,
        callback,
        bcConnection,
        contractNotification
      );
    },
    avswuBlockchainReadLastNotification: async (call, callback) => {
      avswuBlockchainReadLastNotification(
        call,
        callback,
        bcConnection,
        contractNotification
      );
    },
    avswuBlockchainReadLastNotificationList: async (call, callback) => {
      avswuBlockchainReadLastNotificationList(
        call,
        callback,
        bcConnection,
        contractNotification
      );
    }
  });

  // start the server
  server.bindAsync(
    '0.0.0.0:50051',
    grpc.ServerCredentials.createInsecure(),
    (error, result) => {
      if (error) {
        logger.fatal(error);
        process.exit(-1);
      }
      logger.info('starting the gRPC server');
    }
  );
};

main();

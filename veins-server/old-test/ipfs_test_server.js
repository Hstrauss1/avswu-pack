import process from 'node:process';
import logger from './logger_utils.js';

logger.debug('loading grpc...');
// grpc globals
const PROTO_PATH = '/avswu/veins-server/veins_avswu.proto';

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

import {
  ipfsStart,
  avswuIpfsWritePacket,
  avswuIpfsReadPacket
} from './ipfs.js';

const main = async () => {
  // start the grpc server
  logger.debug('start grpc server');
  const server = new grpc.Server();

  // start ipfs
  logger.debug('start ipfs');
  const ipfsStrings = await ipfsStart();
  logger.debug('finish start ipfs');

  // add services
  logger.debug('adding services');
  server.addService(avswuProto.Avswu.service, {
    avswuIpfsWritePacket: async (call, callback) => {
      avswuIpfsWritePacket(call, callback, ipfsStrings);
    },
    avswuIpfsReadPacket: async (call, callback) => {
      avswuIpfsReadPacket(call, callback, ipfsStrings);
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

import { noise } from '@chainsafe/libp2p-noise';
import { yamux } from '@chainsafe/libp2p-yamux';
import { bootstrap } from '@libp2p/bootstrap';
import { identify } from '@libp2p/identify';
import { tcp } from '@libp2p/tcp';
import { MemoryBlockstore } from 'blockstore-core';
import { MemoryDatastore } from 'datastore-core';
import { createHelia } from 'helia';
import { createLibp2p } from 'libp2p';
import { strings } from '@helia/strings';

import logger from './logger_utils.js';

async function createNode() {
  // the blockstore is where we store the blocks that make up files
  const blockstore = new MemoryBlockstore();

  // application-specific data lives in the datastore
  const datastore = new MemoryDatastore();

  // libp2p is the networking layer that underpins Helia
  const libp2p = await createLibp2p({
    datastore,
    addresses: {
      listen: ['/ip4/127.0.0.1/tcp/0']
    },
    transports: [tcp()],
    connectionEncryption: [noise()],
    streamMuxers: [yamux()],
    peerDiscovery: [
      bootstrap({
        list: [
          '/dnsaddr/bootstrap.libp2p.io/p2p/QmNnooDu7bfjPFoTZYxMNLWUQJyrVwtbZg5gBMjTezGAJN',
          '/dnsaddr/bootstrap.libp2p.io/p2p/QmQCU2EcMqAqQPR2i9bChDtGNJchTbq5TbXJJ16u19uLTa',
          '/dnsaddr/bootstrap.libp2p.io/p2p/QmbLHAnMoJPWSCR5Zhtx6BHJX9KiKNN6tpvbUcqanj75Nb',
          '/dnsaddr/bootstrap.libp2p.io/p2p/QmcZf59bWwK5XFi76CZX8cbJ4BhTzzA3gU1ZjYZcYW3dwt'
        ]
      })
    ],
    services: {
      identify: identify()
    }
  });

  return await createHelia({
    datastore,
    blockstore,
    libp2p
  });
}

const main = async () => {
  const helia = await await createNode();
  const ipfsStrings = strings(helia);

  for (let i = 0; i < 20; i++) {
    const cid = await ipfsStrings.add('hello world i=' + i);
    const s = await ipfsStrings.get(cid);
    logger.debug('cid=%s, s=%s', cid, s);
  }
};

main();

const cpabeAddOn = require('./build/Release/cpabeAddOn.node');

// crypto libs
const crypto = require('crypto');

let masterKey;
let publicKey;
let privateKey;
let encryptedData;

const pt = 'hello mello, hello world!';
const policy = '((a1 and a2) and (a3 < 16)) or (a4 > 30)';
const attributes = 'a1, a2, a3 = 15, a4 < 25';

describe('cpabe addon tests', () => {
  test('cpabe setup', () => {
  // test cpabe setup
    const keys = cpabeAddOn.cpabeSetup();
    masterKey = keys.masterKey;
    publicKey = keys.publicKey;

    const publicKeyInt8Array = new Int8Array(publicKey);
    const masterKeyInt8Array = new Int8Array(masterKey);

    // console.log('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%');
    // console.log('publicKeyInt8Array = ', publicKeyInt8Array.length);
    // console.log('masterKeyInt8Array = ', masterKeyInt8Array.length);
    // console.log('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%');

    const pubLength = publicKeyInt8Array.length;
    const actualPub = pubLength;
    const expectedPub = 888;

    expect(actualPub).toBe(expectedPub);

    const masterLength = masterKeyInt8Array.length;
    const actualMasterLength = masterLength;
    const expectedMasterLength = 156;

    expect(actualMasterLength).toBe(expectedMasterLength);
  });

  test('cpabe keygen to create the privateKey', () => {
    const publicKeyInt8Array = new Int8Array(publicKey);
    const masterKeyInt8Array = new Int8Array(masterKey);

    // keygen w/ comma separated attributes

    const privateKeyData = cpabeAddOn.cpabeKeyGen(
        publicKeyInt8Array, publicKeyInt8Array.length,
        masterKeyInt8Array, masterKeyInt8Array.length,
        attributes);

    privateKey = privateKeyData.privateKey;

    const actualHash = crypto.createHash('sha256').update(privateKey.toString()).digest('hex');

    // console.log('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%');
    // console.log('privateKeyData = ', privateKeyData);
    // console.log('bufferHash = ', bufferHash);
    // console.log('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%');

    const expectedHash = '17bf4b46701313ea8fbaf838c24b8647d39bff0a9d2b45f403cb72ba420bd4bd';

    expect(actualHash).toBe(expectedHash);
  });

  test('cpabe encrypt the pt', () => {
    const publicKeyInt8Array = new Int8Array(publicKey);
    const data = Buffer.from(pt);

    const dataInt8Array = new Int8Array(data);
    encryptedData = cpabeAddOn.cpabeEncrypt(
        dataInt8Array, dataInt8Array.length,
        publicKeyInt8Array, publicKeyInt8Array.length,
        policy);

    const cipherBuffer = encryptedData.cipherBufByteArray;
    const actualHash = crypto.createHash('sha256').update(cipherBuffer.toString()).digest('hex');
    console.log('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%');
    console.log('encryptedData = ', encryptedData);
    console.log('actualHash = ', actualHash);
    console.log('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%');

    const expectedHash = '17bf4b46701313ea8fbaf838c24b8647d39bff0a9d2b45f403cb72ba420bd4bd';

    expect(actualHash).toBe(expectedHash);
  });

  test('cpabe decrypt the data', () => {
    const fileLen = encryptedData.fileLen;
    const cipherInt8Array = new Int8Array(encryptedData.cipherBufByteArray);
    const aesInt8Array = new Int8Array(encryptedData.aesBufByteArray);
    const privateKeyInt8Array = new Int8Array(privateKey);
    const publicKeyInt8Array = new Int8Array(publicKey);

    const decryptedRet = cpabeAddOn.cpabeDecrypt(
        fileLen,
        cipherInt8Array, cipherInt8Array.length,
        aesInt8Array, aesInt8Array.length,
        publicKeyInt8Array, publicKeyInt8Array.length,
        privateKeyInt8Array, privateKeyInt8Array.length
    );

    // convert data to pt
    const int8Array = new Int8Array(decryptedRet.fileBufByteArray);
    const ptActual = (Buffer.from(int8Array)).toString();

    // console.log('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%');
    // console.log('decryptedRet = ', decryptedRet);
    // console.log('ptActual = ', ptActual);
    // console.log('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%');

    expect(ptActual).toBe(pt);
  });
});

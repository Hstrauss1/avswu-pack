// contents of index.js
const cpabeAddOn = require('./build/Release/cpabeAddOn.node');

const os = require('os');
if (os.platform() != 'linux') {
  const msg ='ERROR: This code only runs on linux due to CPABE dependencies.';
  throw new Error(msg);
}

console.log('\n\n---------------------------\n\n');

// test cpabe setup
const keys = cpabeAddOn.cpabeSetup();

console.log('keys = ', keys);

const publicKeyInt8Array = new Int8Array(keys.publicKey);
// console.log("publicKeyView8 = ", publicKeyView8);

const masterKeyInt8Array = new Int8Array(keys.masterKey);
// console.log("masterKeyViewInt8 = ", masterKeyViewInt8);


// keygen
// use , to separate and = must have spaces between them (the attribute parsing
// in cpabe libs is terrible...)
let attributes = 'a1, a2, a3 = 15';
attributes = 'a1, a3 = 654321';
// attributes = "a1, a2, a3 = 59";
console.log('attributes = ', attributes);

const privateKeyData = cpabeAddOn.cpabeKeyGen(
    publicKeyInt8Array, publicKeyInt8Array.length,
    masterKeyInt8Array, masterKeyInt8Array.length,
    attributes);

console.log('privateKeyData = ', privateKeyData);

// encrypt
const pt = 'my plain text my plain text my plain text';
const data = Buffer.from(pt);
const policy = '(a1 and a2) or (a3 > 123456)';
// policy = "(a1 and a2)";
console.log('policy = ', policy);

const dataInt8Array = new Int8Array(data);
const encryptedData = cpabeAddOn.cpabeEncrypt(
    dataInt8Array, dataInt8Array.length,
    publicKeyInt8Array, publicKeyInt8Array.length,
    policy);
console.log('encryptedData = ', encryptedData);

// decrypt
const fileLen = encryptedData.fileLen;
const cipherInt8Array = new Int8Array(encryptedData.cipherBufByteArray);
const aesInt8Array = new Int8Array(encryptedData.aesBufByteArray);
const privateKeyInt8Array = new Int8Array(privateKeyData.privateKey);

// console.log("publicKeyInt8Array = ", publicKeyInt8Array);
// console.log("privateKeyInt8Array = ", privateKeyInt8Array);


const decryptedRet = cpabeAddOn.cpabeDecrypt(
    fileLen,
    cipherInt8Array, cipherInt8Array.length,
    aesInt8Array, aesInt8Array.length,
    publicKeyInt8Array, publicKeyInt8Array.length,
    privateKeyInt8Array, privateKeyInt8Array.length
);
console.log('decryptedRet = ', decryptedRet);

// convert data to pt
const int8Array = new Int8Array(decryptedRet.fileBufByteArray);
const ptDecrypted = (Buffer.from(int8Array)).toString();
console.log('ptDecrypted = ', ptDecrypted);

//
//
// try to decrypt w/ bad attributes
//
//

const badAttributes = 'a1, a3 = 10';
const badPrivateKeyData = cpabeAddOn.cpabeKeyGen(
    publicKeyInt8Array, publicKeyInt8Array.length,
    masterKeyInt8Array, masterKeyInt8Array.length,
    badAttributes);

const badPrivateKeyInt8Array = new Int8Array(badPrivateKeyData.privateKey);


const badDecryptedRet = cpabeAddOn.cpabeDecrypt(
    fileLen,
    cipherInt8Array, cipherInt8Array.length,
    aesInt8Array, aesInt8Array.length,
    publicKeyInt8Array, publicKeyInt8Array.length,
    badPrivateKeyInt8Array, badPrivateKeyInt8Array.length
);

console.log('badDecryptedRet = ', badDecryptedRet);

// convert data to pt
const badInt8Array = new Int8Array(badDecryptedRet.fileBufByteArray);
const badPtDecrypted = (Buffer.from(badInt8Array)).toString();
console.log('badPtDecrypted = ', badPtDecrypted);


